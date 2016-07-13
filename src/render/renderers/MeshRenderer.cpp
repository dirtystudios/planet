#include "MeshRenderer.h"
#include "Log.h"
#include <glm/gtx/transform.hpp>
#include "ConstantBuffer.h"
#include "ConstantBufferManager.h"
#include "StateGroupEncoder.h"
#include "DrawItemEncoder.h"

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
const gfx::StateGroup* _base;
const gfx::DrawItem* _item{nullptr};
MeshRenderer::~MeshRenderer() {
    // TODO: Cleanup renderObjs
}

void MeshRenderer::OnInit() {
    gfx::StateGroupEncoder encoder;
    encoder.Begin();
    encoder.SetVertexShader(GetShaderCache()->Get(gfx::ShaderType::VertexShader, "blinn"));
    encoder.SetPixelShader(GetShaderCache()->Get(gfx::ShaderType::PixelShader, "blinn"));
    encoder.SetVertexLayout(GetVertexLayoutCache()->Pos3fNormal3fTex2f());
    _base = encoder.End();

    cb1  = GetConstantBufferManager()->GetConstantBuffer(sizeof(MeshConstants));
    cb2  = GetConstantBufferManager()->GetConstantBuffer(sizeof(MaterialConstants));
    mesh = GetMeshCache()->Get("roxas/roxas.obj");
    mat  = GetMaterialCache()->Get("roxas/roxas.obj", GetShaderCache()->Get(gfx::ShaderType::PixelShader, "blinn"));
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

    if (!_item) {
        gfx::StateGroupEncoder encoder;
        encoder.Begin(_base);
        encoder.BindResource(cb1->GetBinding(1));
        encoder.BindResource(cb2->GetBinding(2));
        encoder.BindTexture(0, mat->diffuseTextures[0]);
        encoder.SetIndexBuffer(mesh->indexBuffer);
        encoder.SetVertexBuffer(mesh->vertexBuffer);
        const gfx::StateGroup* sg = encoder.End();

        gfx::DrawCall drawCall;
        drawCall.type           = gfx::DrawCall::Type::Indexed;
        drawCall.offset         = mesh->indexOffset;
        drawCall.primitiveCount = mesh->indexCount;

        _item = gfx::DrawItemEncoder::Encode(GetRenderDevice(), drawCall, &sg, 1);
        delete sg;
    }    

    MeshConstants* meshBuffer = cb1->Map<MeshConstants>();
    meshBuffer->world = world;
    cb1->Unmap();

    MaterialConstants* materialBuffer = cb2->Map<MaterialConstants>();
    materialBuffer->kd                = mat->KdData;
    materialBuffer->ka                = mat->KaData;
    materialBuffer->ks                = mat->KsData;
    materialBuffer->ke                = mat->KeData;
    materialBuffer->ns = mat->NsData;
    cb2->Unmap();

    renderQueue->AddDrawItem(0, _item);
}
