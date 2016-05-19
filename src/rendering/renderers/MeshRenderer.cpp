#include "MeshRenderer.h"
#include "Log.h"
#include <glm/gtx/transform.hpp>
#include "ConstantBuffer.h"
#include "ConstantBufferManager.h"

Mesh* mesh;
ConstantBuffer* cb;

struct MeshConstants {
    glm::mat4 world;
};
Material* mat;

MeshRenderer::~MeshRenderer() {
    // TODO: Cleanup renderObjs
}

void MeshRenderer::OnInit() {
    graphics::PipelineStateDesc psd;
    psd.vertexShader = GetShaderCache()->Get(graphics::ShaderType::VertexShader, "diffuse2");
    psd.pixelShader  = GetShaderCache()->Get(graphics::ShaderType::PixelShader, "diffuse2");
    psd.vertexLayout = GetVertexLayoutCache()->Pos3fNormal3fTex2f();
    psd.blendState = graphics::BlendState();
    psd.depthState = graphics::DepthState();
    psd.rasterState = graphics::RasterState();
    psd.topology     = graphics::PrimitiveType::Triangles;
    _defaultPS       = GetPipelineStateCache()->Get(psd);    

    cb = GetConstantBufferManager()->GetConstantBuffer(sizeof(MeshConstants));
    mesh = GetMeshCache()->Get("roxas/roxas.obj");
    mat = GetMaterialCache()->Get("roxas/roxas.obj", psd.pixelShader);
}

RenderObj* MeshRenderer::Register(SimObj* simObj) {
    // cant create drawitem at register time since i will need pass and view specific states in the drawitem
    
   return nullptr;
}

void MeshRenderer::Unregister(RenderObj* renderObj) {
    // TODO: Actually remove it and cleanup
}

void MeshRenderer::Submit(RenderQueue* renderQueue, RenderView* renderView) {
    glm::mat4 world = glm::scale(glm::mat4(), glm::vec3(25, 25, 25));


    graphics::DrawItemDesc did;
    did.drawCall.type = graphics::DrawCall::Type::Indexed;
    did.drawCall.offset = mesh->indexOffset;
    did.drawCall.primitiveCount = mesh->indexCount;
    did.pipelineState = _defaultPS;
    
    did.bindingCount = 1;
    did.bindings[0] = cb->GetBinding(1);
    
    did.streamCount = 1;
    did.streams[0].offset = mesh->vertexOffset;
    did.streams[0].stride = mesh->vertexStride;
    did.streams[0].vertexBuffer = mesh->vertexBuffer;
    
    did.indexBuffer = mesh->indexBuffer;
    did.offset = 0;
    
    const graphics::DrawItem* item = GetRenderDevice()->GetDrawItemEncoder()->Encode(did);
    
    cb->Map<MeshConstants>()->world = world;
    cb->Unmap();
    
    renderQueue->AddDrawItem(0, item);
}
/*
    graphics::DrawTask* task = renderQueue->AppendTask(0);

    task->UpdateShaderParam(_transform, &wvp);
    task->UpdateShaderParam(_normTransform, &invWV);

    task->UpdateShaderParam(mat->Ka, &mat->KaData);
    task->UpdateShaderParam(mat->Kd, &mat->KdData);
    task->UpdateShaderParam(mat->Ks, &mat->KsData);
    task->UpdateShaderParam(mat->Ke, &mat->KeData);
    task->UpdateShaderParam(mat->Ns, &mat->NsData);

    task->BindTexture(graphics::ShaderStage::Pixel, mat->diffuseTextures[0], graphics::TextureSlot::Base);

    task->pipelineState = _defaultPS;    
    task->vertexBuffer  = mesh->vertexBuffer;
    task->indexBuffer   = mesh->indexBuffer;
    task->indexCount   = mesh->indexCount;
    task->indexOffset  = mesh->indexOffset;
*/