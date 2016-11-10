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
    float ns;
};
Material*              mat;
const gfx::StateGroup* _base;
const gfx::DrawItem*   _item{nullptr};
MeshRenderer::~MeshRenderer() {
    // TODO: Cleanup renderObjs
}

void MeshRenderer::OnInit() {
    gfx::StateGroupEncoder encoder;
    encoder.Begin();
    encoder.SetVertexShader(services()->shaderCache()->Get(gfx::ShaderType::VertexShader, "blinn"));
    encoder.SetPixelShader(services()->shaderCache()->Get(gfx::ShaderType::PixelShader, "blinn"));
    encoder.SetVertexLayout(services()->vertexLayoutCache()->Pos3fNormal3fTex2f());
    _base = encoder.End();

    cb1  = services()->constantBufferManager()->GetConstantBuffer(sizeof(MeshConstants));
    cb2  = services()->constantBufferManager()->GetConstantBuffer(sizeof(MaterialConstants));
    mesh = services()->meshCache()->Get("roxas/roxas.obj");
    mat  = services()->materialCache()->Get("roxas/roxas.obj", services()->shaderCache()->Get(gfx::ShaderType::PixelShader, "blinn"));
}

void MeshRenderer::Submit(RenderQueue* renderQueue, const FrameView* view) {
    glm::mat4 world = glm::scale(glm::mat4(), glm::vec3(5, 5, 5));

    if (!_item) {
        gfx::StateGroupEncoder encoder;
        encoder.Begin(_base);
        encoder.BindResource(cb1->GetBinding(1));
        encoder.BindResource(cb2->GetBinding(2));
        encoder.BindTexture(0, mat->diffuseTextures[0], gfx::ShaderStageFlags::AllStages);
        encoder.SetIndexBuffer(mesh->indexBuffer);
        encoder.SetVertexBuffer(mesh->vertexBuffer);
        const gfx::StateGroup* sg = encoder.End();

        gfx::DrawCall drawCall;
        drawCall.type           = gfx::DrawCall::Type::Indexed;
        drawCall.offset         = mesh->indexOffset;
        drawCall.primitiveCount = mesh->indexCount;

        std::vector<const gfx::StateGroup*> groups = {sg, renderQueue->defaults};

        _item = gfx::DrawItemEncoder::Encode(device(), drawCall, groups.data(), groups.size());
        delete sg;
    }

    MeshConstants* meshBuffer = cb1->Map<MeshConstants>();
    meshBuffer->world         = world;
    cb1->Unmap();

    MaterialConstants* materialBuffer = cb2->Map<MaterialConstants>();
    materialBuffer->kd                = mat->KdData;
    materialBuffer->ka                = mat->KaData;
    materialBuffer->ks                = mat->KsData;
    materialBuffer->ke                = mat->KeData;
    materialBuffer->ns                = mat->NsData;
    cb2->Unmap();

    renderQueue->AddDrawItem(0, _item);
}
