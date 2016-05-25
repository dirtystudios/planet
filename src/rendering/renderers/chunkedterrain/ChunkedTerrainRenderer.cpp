#include "ChunkedTerrainRenderer.h"
#include "ChunkedTerrain.h"
#include "Spatial.h"
#include "MeshGeneration.h"
#include "ChunkedTerrainVertex.h"
#include "GPUTextureArrayBuffer.h"
#include "Log.h"
#include <glm/gtx/transform.hpp>

ChunkedTerrainRenderer::~ChunkedTerrainRenderer() {
    // TODO: Cleanup renderObjs
}

void ChunkedTerrainRenderer::OnInit() {
    graphics::PipelineStateDesc psd;
    psd.vertexShader = GetShaderCache()->Get(graphics::ShaderType::VertexShader, "diffuse");
    psd.pixelShader  = GetShaderCache()->Get(graphics::ShaderType::PixelShader, "diffuse");
    psd.vertexLayout = GetVertexLayoutCache()->Get(ChunkedTerrainVertex::GetVertexLayoutDesc());
    psd.topology     = graphics::PrimitiveType::Triangles;
    _defaultPS       = GetPipelineStateCache()->Get(psd);
    _transform = GetRenderDevice()->CreateShaderParam(psd.vertexShader, "wvp", graphics::ParamType::Float4x4);
    assert(_defaultPS);
    assert(_transform);
}

RenderObj* ChunkedTerrainRenderer::Register(SimObj* simObj) {
    assert(simObj);
    assert(simObj->HasComponents({ComponentType::ChunkedTerrain, ComponentType::Spatial}));

    ChunkedTerrain* terrain = simObj->GetComponent<ChunkedTerrain>(ComponentType::ChunkedTerrain);
    Spatial* spatial        = simObj->GetComponent<Spatial>(ComponentType::Spatial);

    ChunkedTerrainNode* root = new ChunkedTerrainNode();
    root->lod                = 0;
    root->tx                 = 0;
    root->ty                 = 0;
    root->x                  = spatial->pos.x;
    root->y                  = spatial->pos.y;
    root->size               = terrain->size;

    float halfSize = root->size / 2.f;
    // z-coordinates will be filled in when heightmap data is generated
    root->bbox.max = glm::vec3(halfSize, halfSize, 0);
    root->bbox.min = glm::vec3(-halfSize, -halfSize, 0);

    ChunkedTerrainRenderObj* renderObj = new ChunkedTerrainRenderObj();
    renderObj->size                    = terrain->size;
    renderObj->x                       = spatial->pos.x;
    renderObj->y                       = spatial->pos.y;
    renderObj->heightmapGenerator      = terrain->heightmapGenerator;

    // build mesh to be used for terrain
    renderObj->vertexCount = (kQuadResolution - 1) * (kQuadResolution - 1) * 6;
    std::vector<ChunkedTerrainVertex> vertices;
    vertices.reserve(renderObj->vertexCount);

    meshGen::VertexDelegate vertexGen = [&vertices](float x, float y, float z, float u, float v) {
        vertices.push_back({glm::vec3(x, y, z), glm::vec2(u, v)});
    };

    meshGen::GenerateGrid(glm::vec3(0, 0, 0), glm::vec2(root->size, root->size),
                          glm::uvec2(kQuadResolution, kQuadResolution), vertexGen);
    assert(vertices.size() == renderObj->vertexCount);
//    renderObj->vertexBuffer =
//        GetRenderDevice()->CreateBuffer(graphics::BufferType::VertexBuffer, vertices.data(),
//                                        sizeof(ChunkedTerrainVertex) * vertices.size(), graphics::BufferUsage::Static);
    assert(false); // fix^
    assert(renderObj->vertexBuffer);

    _objs.push_back(renderObj);
    return renderObj;
}

void ChunkedTerrainRenderer::Unregister(RenderObj* renderObj) {
    assert(renderObj->GetRendererType() == RendererType::ChunkedTerrain);
    // TODO: Actually remove it and cleanup
}

void ChunkedTerrainRenderer::Submit(RenderQueue* renderQueue, RenderView* renderView) {
    Camera* cam    = renderView->camera;
    glm::mat4 proj = cam->BuildProjection();
    glm::mat4 view = cam->BuildView();
    glm::mat4 vp   = proj * view;

    for (ChunkedTerrainRenderObj* terrain : _objs) {
//        graphics::DrawTask* task = renderQueue->AppendTask(0);
//
//        task->UpdateShaderParam(_transform, &vp);
//
//        task->pipelineState = _defaultPS;
//        task->vertexBuffer  = terrain->vertexBuffer;
//        task->vertexCount   = terrain->vertexCount;
//        task->vertexOffset  = 0;
    }
}
