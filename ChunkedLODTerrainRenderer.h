#ifndef __chunked_lod_terrain_renderer_h__
#define __chunked_lod_terrain_renderer_h__


#include <map>
#include <vector>
#include "GLHelpers.h"
#include "GPUTileBuffer.h"
#include "LRUTileCache.h"

struct ChunkedLoDTerrainDesc {
    // Note(eugene): hierarchical transforms
    //Transform transform;
    uint32_t size;

    double x;
    double y;

    std::function<double(double x, double y, double z)> heightmap_generator;
};

typedef uint32_t ChunkedLoDTerrainHandle;

class ChunkedLoDTerrainRenderer {
private:
    struct ChunkedLoDVertex { 
        ChunkedLoDVertex(glm::vec2 pos, glm::vec2 tex) : pos(pos), tex(tex) {};
        glm::vec2 pos;
        glm::vec2 tex;
    };

    struct ChunkedLoDTerrainNode {
        uint32_t lod;
        uint32_t tx;
        uint32_t ty;

        double x;
        double y;

        BoundingBox bbox;
        float size;

        ChunkedLoDTerrainNode* children = { NULL };
    };

    struct ChunkedLoDTerrain {
        ChunkedLoDTerrainNode* root;
        GLuint vao;
        GLuint vb;
        std::function<double(double x, double y, double z)> heightmap_generator;
        uint32_t num_vertices;
    };
private:
    const uint32_t TERRAIN_QUAD_RESOLUTION { 64 };
    const float TERRAIN_SPLIT_FACTOR  { 1.5f };
    const uint32_t TERRAIN_MAX_LOD { 7 };
    const uint32_t NUM_QUAD_TREE_NODES  { 4 };
    const uint32_t GPU_TILE_BUFFER_SIZE  { 512 };

    std::map<ChunkedLoDTerrainNode*, ChunkedLoDTerrainDesc> _terrain_descs;
    std::vector<ChunkedLoDTerrain> _terrains;

    GLuint _shaders[2] { 0 };
    GLuint _program { 0 };
    GPUTileBuffer* _gpu_tile_buffer { 0 };
    GPUTileBuffer* _heightmap_normals_buffer { 0 };
    LRUTileCache* _lru_tile_cache { 0 };
    LRUTileCache* _heightmap_normals_cache { 0 };

public:
    ChunkedLoDTerrainRenderer() {
        // Note(eugene): cleanup shaders
        _shaders[0] = gl::CreateShaderFromFile(GL_VERTEX_SHADER, "/Users/eugene.sturm/projects/misc/planet72/shaders/terrain_vs.glsl");
        _shaders[1] = gl::CreateShaderFromFile(GL_FRAGMENT_SHADER, "/Users/eugene.sturm/projects/misc/planet72/shaders/terrain_fs.glsl");
        assert(_shaders[0] && _shaders[1]);
        _program = gl::CreateProgram(_shaders, 2);
        assert(_program);

        _gpu_tile_buffer = new GPUTileBuffer(TERRAIN_QUAD_RESOLUTION, GPU_TILE_BUFFER_SIZE, GL_R32F);
        _lru_tile_cache = new LRUTileCache(_gpu_tile_buffer);
        _heightmap_normals_buffer = new GPUTileBuffer(TERRAIN_QUAD_RESOLUTION, GPU_TILE_BUFFER_SIZE, GL_RGB32F);
        _heightmap_normals_cache = new LRUTileCache(_heightmap_normals_buffer);
    }

    ~ChunkedLoDTerrainRenderer() {
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

    ChunkedLoDTerrainHandle RegisterTerrain(const ChunkedLoDTerrainDesc& desc) {
        ChunkedLoDTerrain terrain = { 0 };
        ChunkedLoDTerrainNode* root = new ChunkedLoDTerrainNode();
        root->lod = 0;
        root->tx = 0;
        root->ty = 0;
        root->x = desc.x;
        root->y = desc.y;
        root->size = desc.size;
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
        BuildVertexGrid<ChunkedLoDVertex>(glm::vec3(root->x, root->y, 1), glm::vec2(root->size, root->size), glm::uvec2(TERRAIN_QUAD_RESOLUTION, TERRAIN_QUAD_RESOLUTION), vertex_generator, &vertices);

        terrain.num_vertices = vertices.size();    
        terrain.vb = gl::CreateBuffer(GL_ARRAY_BUFFER, vertices.data(), sizeof(ChunkedLoDVertex) * vertices.size(), GL_STATIC_DRAW);
        terrain.vao = gl::CreateVertexArrayObject(terrain.vb, { { ParamType::Float2, ParamType::Float2 } });

        terrain.root = root;
        terrain.heightmap_generator = desc.heightmap_generator;

        _terrains.push_back(terrain);

        return _terrains.size() - 1;
    }

    bool UnregisterTerrain(const ChunkedLoDTerrainHandle& handle) {
        return false;
    }

    void Render(Camera& cam, Frustum& frustum) {
        if(_terrains.size() == 0) {
            return;
        }   

        glm::mat4 proj = cam.BuildProjection();
        glm::mat4 view = cam.BuildView();
        glm::mat4 world = glm::mat4();

        uint32_t tex_slot = 0;
        GL_CHECK(glUseProgram(_program));
        gl::SetUniform(_program, "proj", ParamType::Float4x4, &proj);
        gl::SetUniform(_program, "view", ParamType::Float4x4, &view);        
        GL_CHECK(glActiveTexture(GL_TEXTURE0));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D_ARRAY, _gpu_tile_buffer->GetTextureId()));    
        gl::SetUniform(_program, "heightmap_elevations_tile_array", ParamType::Int32, &tex_slot);
        
        tex_slot = 1;
        GL_CHECK(glActiveTexture(GL_TEXTURE1));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D_ARRAY, _heightmap_normals_buffer->GetTextureId()));
        gl::SetUniform(_program, "heightmap_normals_tile_array", ParamType::Int32, &tex_slot);
    

        for(const ChunkedLoDTerrain& terrain : _terrains) {
            GL_CHECK(glBindVertexArray(terrain.vao));

            ChunkedLoDTerrainNode* root = terrain.root;
            std::queue<ChunkedLoDTerrainNode*> dfs_queue;
            dfs_queue.push(root);

            while(!dfs_queue.empty()) {
                ChunkedLoDTerrainNode* node = dfs_queue.front();
                dfs_queue.pop();        

                if(ShouldSplitNode(node, cam.pos)) {
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
                    std::vector<glm::vec3> normal_data;
                    
                    // lambda cant capture static const var
                    uint32_t terrain_quad_resoltuon = TERRAIN_QUAD_RESOLUTION;
                    auto generate_heightmap_func = [&](GPUTile* tile) -> void {
                        float min, max;
                        glm::uvec2 resolution = glm::uvec2(terrain_quad_resoltuon, terrain_quad_resoltuon);
                        GenerateHeightmapRegion(node->lod, glm::vec3(node->x, node->y, 0), glm::vec2(node->size, node->size),
                            resolution, terrain.heightmap_generator, &elevation_data, &max, &min, &normal_data);
                        

                        node->bbox.min.z = min;
                        node->bbox.max.z = max;
                        tile->CopyData(terrain_quad_resoltuon, terrain_quad_resoltuon, GL_RED, elevation_data.data());
                    };
                    

                    std::function<void(GPUTile* tile)> pass_normals_func = [&](GPUTile* tile) -> void {
                        tile->CopyData(terrain_quad_resoltuon, terrain_quad_resoltuon, GL_RGB, normal_data.data());
                    };

                    Tile* elevations_tile = _lru_tile_cache->Get(node->lod, node->tx, node->ty, generate_heightmap_func);
                    Tile* normals_tile = _heightmap_normals_cache->Get(node->lod, node->tx, node->ty, pass_normals_func);

                    //if(!frustum.IsBoxInFrustum(node->bbox)) {
                    //    continue;
                    //}
        
                    float scale_factor = node->size / (node->size * pow(2, (node->lod)));
                    glm::mat4 translation = glm::translate(glm::mat4(), glm::vec3(node->x, node->y, 0));            
                    glm::mat4 scale = glm::scale(glm::vec3(scale_factor, scale_factor, 1.f));
                    world = translation * scale;
                    int elevations_tile_index = elevations_tile->data->index;
                    int normals_tile_index = normals_tile->data->index;
                    gl::SetUniform(_program, "elevations_tile_index", ParamType::Int32, &elevations_tile_index);
                    gl::SetUniform(_program, "normals_tile_index", ParamType::Int32, &normals_tile_index);
                    gl::SetUniform(_program, "world", ParamType::Float4x4, &world);
                            
                    GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, terrain.num_vertices));
                }

            }
        }
    }
private:
    template <class T>
    void BuildVertexGrid(
        float center_x, float center_y, float center_z, 
        float size_x, float size_y, 
        uint32_t resolution_x, uint32_t resolution_y, 
        std::function<T(float x, float y, float u, float v)> vertex_generator, 
        std::vector<T>* vertices) 
    {
        float half_size_x = size_x / 2.f;
        float half_size_y = size_y / 2.f;
        
        float dx = size_x / (float)(resolution_x - 1);
        float dy = size_y / (float)(resolution_y - 1);

        auto generate_vertex = [&] (uint32_t i, uint32_t j) -> T { 
            float x = center_x - half_size_x + (j * dx);
            float y = center_y + half_size_y - (i * dy);
            float u = j * dx / size_x;
            float v = i * dy / size_y;
            
            return vertex_generator(x, y, u, v); 
        };

        for(uint32_t i = 0; i < resolution_y - 1; ++i) {
            for(uint32_t j = 0; j < resolution_x - 1; ++j) {
                vertices->push_back(generate_vertex(i, j));
                vertices->push_back(generate_vertex(i+1, j));
                vertices->push_back(generate_vertex(i+1, j+1));

                vertices->push_back(generate_vertex(i+1, j+1));
                vertices->push_back(generate_vertex(i, j+1));
                vertices->push_back(generate_vertex(i, j));
            }
        }
    }  

    template <class T>
    void BuildVertexGrid(const glm::vec3& center, const glm::vec2& size, const glm::uvec2& resolution,     
        std::function<T(float x, float y, float u, float v)> vertex_generator, 
        std::vector<T>* vertices) 
    {
        BuildVertexGrid(center.x, center.y, center.z, size.x, size.y, resolution.x, resolution.y,
            vertex_generator, vertices);
    }


    /*
        @param region_center Center point or region to generate coordinates from
        @param region_size Size of region to generate
        @param resolution Resolution to sample for generating heightmap values
        @param heightmap_generator Function to call to generate a heightmap value for a give coordinate
        @param data Output buffer to store heightmap values in
        @param max Output parameter to store max height value
        @param min Output parameter to store min height value
    */
    void GenerateHeightmapRegion(const glm::dvec3& region_center, 
        const glm::vec2& region_size, 
        const glm::uvec2& resolution, 
        std::function<float(double x, double y, double z)> heightmap_generator, 
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

    void GenerateHeightmapRegionNormals(const std::vector<float>& heightmap_data, 
        const glm::vec2& size, 
        const glm::uvec2& resolution, 
        uint32_t lod,
        std::vector<glm::vec3>* generated_normal_data) {
        
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
                glm::vec3 normal = glm::normalize(glm::vec3(scale * x, scale * y, z));

                generated_normal_data->push_back(normal);
            }
        }

    }

    void GenerateHeightmapRegion(uint32_t lod, const glm::vec3& center, const glm::vec2& size, const glm::uvec2& resolution,
                                 std::function<float(double x, double y, double z)> heightmap_generator,
                                 std::vector<float>* elevation_data, float* elevation_max, float* elevation_min,
                                 std::vector<glm::vec3>* normal_data) {    
        GenerateHeightmapRegion(center, size, resolution, heightmap_generator, elevation_data, elevation_max, elevation_min);
        GenerateHeightmapRegionNormals(*elevation_data, size, resolution, lod, normal_data);
    }
    bool ShouldSplitNode(const ChunkedLoDTerrainNode* node, const glm::vec3& eye) {
        if(node->lod >= TERRAIN_MAX_LOD) {
            return false;
        }

        return ComputeScreenSpaceError(node->bbox, eye, node->size);
    }

    bool ComputeScreenSpaceError(const BoundingBox& bbox, const glm::vec3& eye, float size) {
        double d = bbox.GetDistanceFromBoundingBox(eye);            
        if(d < TERRAIN_SPLIT_FACTOR * size) {
            return true;
        }

        return false;
    }
    
    /*
    double ComputeScreenSpaceError(const BoundingBox& bbox, const glm::vec3& eye, float hfov, uint32_t viewport_width, float geometric_error) {
        double d = bbox.GetDistanceFromBoundingBox(eye);            

        double w = 2.f * d * tan(hfov / 2.f);
        if(w == 0) {
            return 0; //TODO: this mean if we are ever inside the bounding box, we say we have 0 screen space error. NOT good.
        }
        double res = geometric_error * viewport_width / w;        
        return res;   
    }
    */
};

#endif