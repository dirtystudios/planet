#include "ChunkedLoDTerrainRenderer.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "Config.h"
#include "ConsoleCommands.h"
#include "File.h"
#include "Log.h"
#include <queue>
#include "Helpers.h"


ChunkedLoDTerrainRenderer::ChunkedLoDTerrainRenderer(graphics::RenderDevice* render_device) : _render_device(render_device) {
    config::ConsoleCommands::getInstance().RegisterCommand(
        "ToggleWireFrameMode", BIND_MEM_CB(&ChunkedLoDTerrainRenderer::ToggleWireFrameMode, this));

    std::string shaderDirPath = config::Config::getInstance().GetConfigString("RenderDeviceSettings", "ShaderDirectory");
    if (!fs::IsPathDirectory(shaderDirPath)) {
        LOG_D("%s","Invalid Directory Path given for ShaderDirectory. Attempting default.");
        shaderDirPath = fs::AppendPathProcessDir("/shaders");
    }

    std::string vs_contents = ReadFileContents(shaderDirPath +"/" + render_device->DeviceConfig.DeviceAbbreviation + "/terrain_vs" + render_device->DeviceConfig.ShaderExtension);
    std::string fs_contents = ReadFileContents(shaderDirPath + "/" + render_device->DeviceConfig.DeviceAbbreviation + "/terrain_fs" + render_device->DeviceConfig.ShaderExtension);

    // Note(eugene): cleanup shaders
    _shaders[0] = _render_device->CreateShader(graphics::ShaderType::VERTEX_SHADER, vs_contents);
    _shaders[1] = _render_device->CreateShader(graphics::ShaderType::FRAGMENT_SHADER, fs_contents);

    assert(_shaders[0] && _shaders[1]);

    graphics::TextureHandle heightmap_texture_array = _render_device->CreateTextureArray(graphics::TextureFormat::R32F, 1, TERRAIN_QUAD_RESOLUTION, TERRAIN_QUAD_RESOLUTION, GPU_TILE_BUFFER_SIZE);
    graphics::TextureHandle normalmap_texture_array = _render_device->CreateTextureArray(graphics::TextureFormat::RGBA32F, 1, TERRAIN_QUAD_RESOLUTION, TERRAIN_QUAD_RESOLUTION, GPU_TILE_BUFFER_SIZE);
    assert(heightmap_texture_array && normalmap_texture_array);

    _gpu_tile_buffer = new GPUTileBuffer(TERRAIN_QUAD_RESOLUTION, GPU_TILE_BUFFER_SIZE, heightmap_texture_array);
    _lru_tile_cache = new LRUTileCache(_gpu_tile_buffer);
    _heightmap_normals_buffer = new GPUTileBuffer(TERRAIN_QUAD_RESOLUTION, GPU_TILE_BUFFER_SIZE, normalmap_texture_array);
    _heightmap_normals_cache = new LRUTileCache(_heightmap_normals_buffer);
}


ChunkedLoDTerrainRenderer::~ChunkedLoDTerrainRenderer() {
    if(_gpu_tile_buffer) {
        delete _gpu_tile_buffer;
    }
    if(_heightmap_normals_buffer) {
        delete _heightmap_normals_buffer;
    }
    if(_lru_tile_cache) {
        delete _lru_tile_cache;
    }
    if(_heightmap_normals_cache) {
        delete _heightmap_normals_cache;
    }
    // Note(eugene): cleanup program, meshes and terrains
}

ChunkedLoDTerrainHandle ChunkedLoDTerrainRenderer::RegisterTerrain(const ChunkedLoDTerrainDesc& desc) {
    ChunkedLoDTerrain terrain = { 0 };
    ChunkedLoDTerrainNode* root = new ChunkedLoDTerrainNode();
    root->lod = 0;
    root->tx = 0;
    root->ty = 0;
    root->x = desc.x;
    root->y = desc.y;
    root->size = desc.size;
    terrain.size = desc.size;
    terrain.translation = desc.translation;
    terrain.rotation = desc.rotation;
    float half_size = root->size / 2.f;
    // z-coordinates will be filled in when heightmap data is generated
    root->bbox.max = glm::vec3(half_size, half_size, 0);
    root->bbox.min = glm::vec3(-half_size, -half_size, 0);

    // build mesh to be used for terrain
    auto vertex_generator = [&](float x, float y, float u, float v) -> ChunkedLoDVertex  {
        return ChunkedLoDVertex(glm::vec2(x, y), glm::vec2(u, v));
    };

    uint32_t t = TERRAIN_QUAD_RESOLUTION;
    std::vector<ChunkedLoDVertex> vertices;
    BuildVertexGrid<ChunkedLoDVertex>(glm::vec3(0, 0, 0), glm::vec2(root->size, root->size), glm::uvec2(TERRAIN_QUAD_RESOLUTION, TERRAIN_QUAD_RESOLUTION), vertex_generator, &vertices);

    terrain.num_vertices = vertices.size();
    graphics::VertLayout layout;
    layout.Add(graphics::ParamType::Float2);
    layout.Add(graphics::ParamType::Float2);
    terrain.vb = _render_device->CreateVertexBuffer(layout, vertices.data(), sizeof(ChunkedLoDVertex) * vertices.size(), graphics::BufferUsage::STATIC);

    terrain.root = root;
    terrain.heightmap_generator = desc.heightmap_generator;

    _terrains.push_back(terrain);

    return _terrains.size() - 1;
}

bool ChunkedLoDTerrainRenderer::UnregisterTerrain(const ChunkedLoDTerrainHandle& handle) {
    return false;
}

void ChunkedLoDTerrainRenderer::Render(Camera* cam, Frustum* frustum) {
    if(_terrains.size() == 0) {
        return;
    }


    glm::mat4 proj = cam->BuildProjection();
    glm::mat4 view = cam->BuildView();
    glm::mat4 world = glm::mat4();

    _render_device->SetVertexShader(_shaders[0]);
    _render_device->SetPixelShader(_shaders[1]);
    _render_device->SetShaderParameter(_shaders[0], graphics::ParamType::Float4x4, "proj", &proj);
    _render_device->SetShaderParameter(_shaders[0], graphics::ParamType::Float4x4, "view", &view);
    _render_device->SetShaderTexture(_shaders[0], _gpu_tile_buffer->GetTextureId(), graphics::TextureSlot::BASE);
    _render_device->SetShaderTexture(_shaders[0], _heightmap_normals_buffer->GetTextureId(), graphics::TextureSlot::NORMAL);

    graphics::RasterState wireFrameState;
    wireFrameState.fill_mode = graphics::FillMode::WIREFRAME;

    graphics::RasterState defaultFillState = graphics::RasterState();

    for(const ChunkedLoDTerrain& terrain : _terrains) {

        ChunkedLoDTerrainNode* root = terrain.root;
        std::queue<ChunkedLoDTerrainNode*> dfs_queue;
        dfs_queue.push(root);

        while(!dfs_queue.empty()) {
            ChunkedLoDTerrainNode* node = dfs_queue.front();
            dfs_queue.pop();

            if(ShouldSplitNode(node, cam->pos)) {
                if(!node->children) {
                    float child_size = node->size / 2.f;
                    float half_child_size = child_size / 2.f;
                    double child_pos[4][2] = {
                        { node->x - half_child_size, node->y + half_child_size },
                        { node->x + half_child_size, node->y + half_child_size },
                        { node->x - half_child_size, node->y - half_child_size },
                        { node->x + half_child_size, node->y - half_child_size },
                    };

                    node->children = new ChunkedLoDTerrainNode[NUM_QUAD_TREE_NODES];
                    for(uint32_t i = 0; i < NUM_QUAD_TREE_NODES; ++i) {
                        ChunkedLoDTerrainNode* child = &node->children[i];

                        child->x = child_pos[i][0];
                        child->y = child_pos[i][1];
                        child->size = child_size;
                        child->lod = node->lod + 1;
                        child->tx = node->tx * 2 + (i % 2 == 0 ? 0 : 1);
                        child->ty = node->ty * 2 + (i < 2 ? 1 : 0);
                        child->bbox.max = glm::vec3(child->x + half_child_size, child->y + half_child_size, 0);
                        child->bbox.min = glm::vec3(child->x - half_child_size, child->y - half_child_size, 0);
                    }
                }

                for(uint32_t idx = 0; idx < NUM_QUAD_TREE_NODES; ++idx) {
                    dfs_queue.push(&node->children[idx]);
                }
            } else {
                std::vector<float> elevation_data;
                std::vector<glm::vec4> normal_data;

                // lambda cant capture static const var
                uint32_t terrain_quad_resolution = TERRAIN_QUAD_RESOLUTION;
                auto generate_heightmap_func = [&](GPUTile* tile) -> void {
                    float min, max;
                    glm::uvec2 resolution = glm::uvec2(terrain_quad_resolution, terrain_quad_resolution);
                    GenerateHeightmapRegion(node->lod, glm::vec3(node->x, node->y, 0), glm::vec2(node->size, node->size),
                        resolution, terrain.heightmap_generator, &elevation_data, &max, &min, &normal_data);


                    node->bbox.min.z = min;
                    node->bbox.max.z = max;
                    //tile->CopyData(terrain_quad_resolution, terrain_quad_resolution, GL_RED, elevation_data.data());
                    _render_device->UpdateTextureArray(tile->texture_array_id, tile->index, terrain_quad_resolution, 
                        terrain_quad_resolution, elevation_data.data());
                };


                std::function<void(GPUTile* tile)> pass_normals_func = [&](GPUTile* tile) -> void {
                    //tile->CopyData(terrain_quad_resolution, terrain_quad_resolution, GL_RGB, normal_data.data());
                    _render_device->UpdateTextureArray(tile->texture_array_id, tile->index, terrain_quad_resolution, 
                        terrain_quad_resolution, normal_data.data());
                };

                Tile* elevations_tile = _lru_tile_cache->Get(node->lod, node->tx, node->ty, generate_heightmap_func);
                Tile* normals_tile = _heightmap_normals_cache->Get(node->lod, node->tx, node->ty, pass_normals_func);

                //if(!frustum.IsBoxInFrustum(node->bbox)) {
                //    continue;
                //}

                float scale_factor1 = node->size;
                float scale_factor2 = node->size / (node->size * pow(2, (node->lod)));
                glm::mat4 translation = glm::translate(glm::mat4(), glm::vec3(node->x, node->y, 0));
                glm::mat4 scale1 = glm::scale(glm::vec3(scale_factor1, scale_factor1, 1.f));
                glm::mat4 scale2 = glm::scale(glm::vec3(scale_factor2, scale_factor2, 1.f));
                world = terrain.rotation;
                int elevations_tile_index = elevations_tile->data->index;
                int normals_tile_index = normals_tile->data->index;
            

                _render_device->SetShaderParameter(_shaders[0], graphics::ParamType::Float, "rradius", &(*(float*)&terrain.size));
                _render_device->SetShaderParameter(_shaders[0], graphics::ParamType::Int32, "elevations_tile_index", &elevations_tile_index);
                _render_device->SetShaderParameter(_shaders[0], graphics::ParamType::Int32, "normals_tile_index", &normals_tile_index);
                _render_device->SetShaderParameter(_shaders[0], graphics::ParamType::Float4x4, "world", &world);
                _render_device->SetShaderParameter(_shaders[0], graphics::ParamType::Float4x4, "trans", &translation);
                _render_device->SetShaderParameter(_shaders[0], graphics::ParamType::Float4x4, "scale", &scale2);
                _render_device->SetVertexBuffer(terrain.vb);
                if (m_wireFrameMode)
                    _render_device->SetRasterState(wireFrameState);
                else
                    _render_device->SetRasterState(defaultFillState);
                _render_device->DrawPrimitive(graphics::PrimitiveType::TRIANGLES, 0, terrain.num_vertices);
            }
        }
    }
}

void ChunkedLoDTerrainRenderer::GenerateHeightmapRegion(const glm::dvec3& region_center,
    const glm::vec2& region_size,
    const glm::uvec2& resolution,
    std::function<double(double x, double y, double z)> heightmap_generator,
    std::vector<float>* data, float* max, float* min) {

    *max = std::numeric_limits<float>::min();
    *min = std::numeric_limits<float>::max();

    glm::vec2 half_size = region_size / 2.f;
    float dx = region_size.x / (float)(resolution.x - 1);
    float dy = region_size.y / (float)(resolution.y - 1);

    for(uint32_t i = 0; i < resolution.y; ++i) {
        for(uint32_t j = 0; j < resolution.x; ++j) {
            double x = region_center.x - half_size.x + (j * dx);
            double y = region_center.y + half_size.y - (i * dy);

            float val = heightmap_generator(x, y, 0);

            if(val > *max) {
                *max = val;
            } else if(val < *min) {
                *min = val;
            }

            data->push_back(val);
        }
    }
}

void ChunkedLoDTerrainRenderer::GenerateHeightmapRegionNormals(const std::vector<float>& heightmap_data,
    const glm::vec2& size,
    const glm::uvec2& resolution,
    uint32_t lod,
    std::vector<glm::vec4>* generated_normal_data) {

    // convert from 2D index to 1D index, clamping to edges (ex. i=-1 => i=0)
    auto get_index = [&](int32_t i, int32_t j) -> int32_t {
        int32_t k = (i >= (int32_t)resolution.x) ? resolution.x - 1 : ((i < 0) ? 0 : i);
        int32_t p = (j >= (int32_t)resolution.y) ? resolution.y - 1 : ((j < 0) ? 0 : j);
       // LOG_D("%d %d %d -> %d %d", j >= resolution.y, i, j, k, p);
        return k * resolution.y + p;
    };

    // Note(eugene): need to do this on gpu
    for(uint32_t i = 0; i < resolution.x; ++i) {
        for(uint32_t j = 0; j < resolution.y; ++j) {
            float tl = heightmap_data[get_index(i - 1, j - 1)];
            float t  = heightmap_data[get_index(i - 1, j)];
            float tr = heightmap_data[get_index(i - 1, j + 1)];
            float r  = heightmap_data[get_index(i, j + 1)];
            float br = heightmap_data[get_index(i + 1, j + 1)];
            float b  = heightmap_data[get_index(i + 1, j)];
            float bl = heightmap_data[get_index(i + 1, j - 1)];
            float l  = heightmap_data[get_index(i, j - 1)];

            // Note(eugene): not sure how to actually do this correctly
            //
            // There is an issue that depending on the terrain size or heightmap we set, the z
            // value chosen will be proportionally different that the x and y values that are
            // be computed. By scaling the x and y values by this term, x and y values will be
            // proportionally similar to the z value no matter the terrain size or resolution.
            //
            float scale = ((float)resolution.x / (float)(size.x * pow(2, lod))) / 0.032f;

            // This needs to scale with region LoD to keep the ratio between size of z value and
            // size of x/y values similar. If we don't, the z value will dominate more an more in
            // the normal as we go to higher LoD regions.
            float z = 1.f / pow(2, (lod));

            float x = -((br - bl) + (2.f * (r - l)) + (tr - tl));
            float y = -((tl - bl) + (2.f * (t - b)) + (tr - br));
            glm::vec4 normal = glm::normalize(glm::vec4(scale * x, scale * y, z, 0.0f));

            generated_normal_data->push_back(normal);
        }
    }

}

void ChunkedLoDTerrainRenderer::GenerateHeightmapRegion(uint32_t lod, const glm::vec3& center, const glm::vec2& size, const glm::uvec2& resolution,
                             std::function<double(double x, double y, double z)> heightmap_generator,
                             std::vector<float>* elevation_data, float* elevation_max, float* elevation_min,
                             std::vector<glm::vec4>* normal_data) {
    GenerateHeightmapRegion(center, size, resolution, heightmap_generator, elevation_data, elevation_max, elevation_min);
    GenerateHeightmapRegionNormals(*elevation_data, size, resolution, lod, normal_data);
}
bool ChunkedLoDTerrainRenderer::ShouldSplitNode(const ChunkedLoDTerrainNode* node, const glm::vec3& eye) {
    if(node->lod >= TERRAIN_MAX_LOD) {
        return false;
    }

    return ComputeScreenSpaceError(node->bbox, eye, node->size);
}

bool ChunkedLoDTerrainRenderer::ComputeScreenSpaceError(const BoundingBox& bbox, const glm::vec3& eye, float size) {
    double d = bbox.GetDistanceFromBoundingBox(eye);
    if(d < TERRAIN_SPLIT_FACTOR * size) {
        return true;
    }

    return false;
}

/*
double ChunkedLoDTerrainRenderer::ComputeScreenSpaceError(const BoundingBox& bbox, const glm::vec3& eye, float hfov, uint32_t viewport_width, float geometric_error) {
    double d = bbox.GetDistanceFromBoundingBox(eye);

    double w = 2.f * d * tan(hfov / 2.f);
    if(w == 0) {
        return 0; //TODO: this mean if we are ever inside the bounding box, we say we have 0 screen space error. NOT good.
    }
    double res = geometric_error * viewport_width / w;
    return res;
}
*/
