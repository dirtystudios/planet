#include "MeshRenderer.h"
#include "Log.h"
#include <glm/gtx/transform.hpp>
#include "ConstantBuffer.h"
#include "ConstantBufferManager.h"

Mesh* mesh;
ConstantBuffer* cb1;
ConstantBuffer* cb2;

struct MeshConstants {
    glm::mat4 world;
};

struct MaterialConstants {
    glm::vec3 ka;
    float padding0;
    glm::vec3 kd;
    float padding1;
    glm::vec3 ks;
    float padding2;
    glm::vec3 ke;
    float padding3;
    float ns;
};
Material* mat;

MeshRenderer::~MeshRenderer() {
    // TODO: Cleanup renderObjs
}

void MeshRenderer::OnInit() {
    graphics::PipelineStateDesc psd;
    psd.vertexShader = GetShaderCache()->Get(graphics::ShaderType::VertexShader, "blinn");
    psd.pixelShader  = GetShaderCache()->Get(graphics::ShaderType::PixelShader, "blinn");
    psd.vertexLayout = GetVertexLayoutCache()->Pos3fNormal3fTex2f();
    psd.topology     = graphics::PrimitiveType::Triangles;
    _defaultPS       = GetPipelineStateCache()->Get(psd);

    cb1  = GetConstantBufferManager()->GetConstantBuffer(sizeof(MeshConstants));
    cb2  = GetConstantBufferManager()->GetConstantBuffer(sizeof(MaterialConstants));
    mesh = GetMeshCache()->Get("roxas/roxas.obj");
    mat  = GetMaterialCache()->Get("roxas/roxas.obj", psd.pixelShader);
}

RenderObj* MeshRenderer::Register(SimObj* simObj) {
    // cant create drawitem at register time since i will need pass and view specific states in the drawitem

    return nullptr;
}

void MeshRenderer::Unregister(RenderObj* renderObj) {
    // TODO: Actually remove it and cleanup
}

void MeshRenderer::Submit(RenderQueue* renderQueue, RenderView* renderView) {
    glm::mat4 world = glm::scale(glm::mat4(), glm::vec3(5, 5, 5));

    graphics::DrawItemDesc did;
    did.drawCall.type           = graphics::DrawCall::Type::Indexed;
    did.drawCall.offset         = mesh->indexOffset;
    did.drawCall.primitiveCount = mesh->indexCount;
    did.pipelineState           = _defaultPS;

    did.bindingCount = 3;
    did.bindings[0]  = cb1->GetBinding(1);

    did.bindings[1].type     = graphics::Binding::Type::Texture;
    did.bindings[1].slot = 0;
    did.bindings[1].resource = mat->diffuseTextures[0];

    did.bindings[2] = cb2->GetBinding(2);

    did.streamCount             = 1;
    did.streams[0].offset       = mesh->vertexOffset;
    did.streams[0].stride       = mesh->vertexStride;
    did.streams[0].vertexBuffer = mesh->vertexBuffer;

    did.indexBuffer = mesh->indexBuffer;
    did.offset      = 0;

    const graphics::DrawItem* item = GetRenderDevice()->GetDrawItemEncoder()->Encode(did);

    MeshConstants* meshBuffer = cb1->Map<MeshConstants>();
    meshBuffer->world = world;
    cb1->Unmap();

    MaterialConstants* materialBuffer = cb2->Map<MaterialConstants>();
    materialBuffer->kd                = mat->KdData;
    materialBuffer->ka                = mat->KaData;
    materialBuffer->ks = mat->KsData;
    materialBuffer->ke = mat->KeData;
    materialBuffer->ns = mat->NsData;
    cb2->Unmap();
    
    
    renderQueue->AddDrawItem(0, item);
}
