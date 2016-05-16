#include "MeshRenderer.h"
#include "Log.h"
#include <glm/gtx/transform.hpp>

Mesh* mesh;

MeshRenderer::~MeshRenderer() {
    // TODO: Cleanup renderObjs
}

void MeshRenderer::OnInit() {
    graphics::PipelineStateDesc psd;
    psd.vertexShader = GetShaderCache()->Get(graphics::ShaderType::VertexShader, "diffuse2");
    psd.pixelShader  = GetShaderCache()->Get(graphics::ShaderType::PixelShader, "diffuse2");
    psd.vertexLayout = GetVertexLayoutCache()->Pos3fNormal3f();
    psd.blendState = graphics::BlendState();
    psd.depthState = graphics::DepthState();
    psd.rasterState = graphics::RasterState();
    psd.topology     = graphics::PrimitiveType::Triangles;    
    //psd.rasterState.fillMode = graphics::FillMode::Wireframe;
    _defaultPS       = GetPipelineStateCache()->Get(psd);
    _transform = GetRenderDevice()->CreateShaderParam(psd.vertexShader, "wvp", graphics::ParamType::Float4x4);
    _normTransform = GetRenderDevice()->CreateShaderParam(psd.vertexShader, "invWV", graphics::ParamType::Float4x4);
    assert(_defaultPS);
    assert(_transform);

    
    mesh = GetMeshCache()->Get("roxas/roxas.obj");
}

RenderObj* MeshRenderer::Register(SimObj* simObj) {
   return nullptr;
}

void MeshRenderer::Unregister(RenderObj* renderObj) {    
    // TODO: Actually remove it and cleanup
}

void MeshRenderer::Submit(RenderQueue* renderQueue, RenderView* renderView) {
    Camera* cam    = renderView->camera;
    glm::mat4 proj = cam->BuildProjection();
    glm::mat4 view = cam->BuildView();
    glm::mat4 world = glm::scale(glm::mat4(), glm::vec3(25, 25, 25));
    glm::mat4 wvp   = proj * view * world;
    glm::mat4 invWV = glm::transpose(glm::inverse(view * world));

    
    graphics::DrawTask* task = renderQueue->AppendTask(0);

    task->UpdateShaderParam(_transform, &wvp);
    task->UpdateShaderParam(_normTransform, &invWV);

    task->pipelineState = _defaultPS;    
    task->vertexBuffer  = mesh->vertexBuffer;
    task->indexBuffer   = mesh->indexBuffer;
    task->indexCount   = mesh->indexCount;
    task->indexOffset  = mesh->indexOffset;
}