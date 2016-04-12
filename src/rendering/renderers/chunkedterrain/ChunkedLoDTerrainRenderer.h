
#ifndef __chunked_lod_terrain_renderer_h__
#define __chunked_lod_terrain_renderer_h__

#include "RenderDevice.h"
#include <map>
#include <functional>
#include <algorithm>
#include "GPUTileBuffer.h"
#include "LRUTileCache.h"
#include "glm/glm.hpp"
#include "BoundingBox.h"
#include "Camera.h"
#include "Frustum.h"
#include "Renderer.h"
#include "Spatial.h"
#include "ChunkedTerrain.h"
#include "SimObj.h"

struct ChunkedLoDTerrainDesc {
    // Note(eugene): hierarchical transforms
    // Transform transform;
    uint32_t size;

    double x;
    double y;

    std::function<double(double x, double y, double z)> heightmap_generator;
};

typedef uint32_t ChunkedLoDTerrainHandle;

class ChunkedLoDTerrainRenderer : public Renderer {
private:
    struct ChunkedLoDVertex {
        ChunkedLoDVertex(glm::vec2 pos, glm::vec2 tex) : pos(pos), tex(tex) {};
        glm::vec2 pos;
        glm::vec2 tex;
    };

    struct ChunkedTerrainRenderObj : public RenderObj {
        ChunkedTerrainRenderObj() : RenderObj(RendererType::ChunkedTerrain) {};
        uint32_t handle;
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
        graphics::VertexBufferHandle vb;
        std::function<double(double x, double y, double z)> heightmap_generator;
        uint32_t num_vertices;
    };

    const uint32_t TERRAIN_QUAD_RESOLUTION { 64 };
    const float TERRAIN_SPLIT_FACTOR  { 1.5f };
    const uint32_t TERRAIN_MAX_LOD { 7 };
    const uint32_t NUM_QUAD_TREE_NODES  { 4 };
    const uint32_t GPU_TILE_BUFFER_SIZE  { 512 };

    std::map<ChunkedLoDTerrainNode*, ChunkedLoDTerrainDesc> _terrain_descs;
    std::vector<ChunkedLoDTerrain> _terrains;

    graphics::ShaderHandle _shaders[2] { 0 };
    graphics::ProgramHandle _program { 0 };

    GPUTileBuffer* _gpu_tile_buffer { 0 };
    GPUTileBuffer* _heightmap_normals_buffer { 0 };
    LRUTileCache* _lru_tile_cache { 0 };
    LRUTileCache* _heightmap_normals_cache { 0 };

    graphics::RenderDevice* _render_device;

    bool m_wireFrameMode = false;

    std::vector<ChunkedTerrainRenderObj*> _objs;
public:
    ChunkedLoDTerrainRenderer(graphics::RenderDevice* render_device);
    ~ChunkedLoDTerrainRenderer();

    ChunkedLoDTerrainHandle RegisterTerrain(const ChunkedLoDTerrainDesc& desc);
    bool UnregisterTerrain(const ChunkedLoDTerrainHandle& handle);
    void Render(Camera* cam, Frustum* frustum);

    // need dummy arg for inputCallback Definition
    bool ToggleWireFrameMode(float notUsed) { m_wireFrameMode = !m_wireFrameMode; return true; }



    RenderObj* Register(SimObj* simObj) final {         
        assert(simObj->HasComponents({ ComponentType::ChunkedTerrain, ComponentType::Spatial }));        
        ChunkedTerrain* terrain = simObj->GetComponent<ChunkedTerrain>(ComponentType::ChunkedTerrain);
        Spatial* spatial = simObj->GetComponent<Spatial>(ComponentType::Spatial);        



        ChunkedLoDTerrainDesc desc;
        desc.size = terrain->size;
        desc.x = spatial->pos.x;
        desc.y = spatial->pos.y;
        desc.heightmap_generator = terrain->heightmapGenerator;

        ChunkedTerrainRenderObj* renderObj = new ChunkedTerrainRenderObj();
        renderObj->handle = RegisterTerrain(desc);

        _objs.push_back(renderObj);
        return renderObj;
    }

    void Unregister(RenderObj* renderObj) final {
        assert(renderObj->GetRendererType() == RendererType::ChunkedTerrain);
        _objs.erase(std::remove(begin(_objs), end(_objs), static_cast<ChunkedTerrainRenderObj*>(renderObj)), end(_objs));
    }

    void Submit(RenderView* renderView) final {
        Render(renderView->camera, nullptr);
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
        std::vector<T>* vertices) {

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
        std::function<double(double x, double y, double z)> heightmap_generator,
        std::vector<float>* data, float* max, float* min);

    void GenerateHeightmapRegionNormals(const std::vector<float>& heightmap_data,
        const glm::vec2& size,
        const glm::uvec2& resolution,
        uint32_t lod,
        std::vector<glm::vec4>* generated_normal_data);

    void GenerateHeightmapRegion(uint32_t lod, const glm::vec3& center, const glm::vec2& size, const glm::uvec2& resolution,
                                 std::function<double(double x, double y, double z)> heightmap_generator,
                                 std::vector<float>* elevation_data, float* elevation_max, float* elevation_min,
                                 std::vector<glm::vec4>* normal_data);

    bool ShouldSplitNode(const ChunkedLoDTerrainNode* node, const glm::vec3& eye);
    bool ComputeScreenSpaceError(const BoundingBox& bbox, const glm::vec3& eye, float size);
    //double ComputeScreenSpaceError(const BoundingBox& bbox, const glm::vec3& eye, float hfov, uint32_t viewport_width, float geometric_error);

};

#endif
