#include "MeshRenderer.h"
#include "Log.h"
#include <glm/gtx/transform.hpp>
#include "ConstantBuffer.h"
#include "ConstantBufferManager.h"
#include "StateGroupEncoder.h"
#include "MeshGeometry.h"
#include "DrawItemEncoder.h"
#include "Image.h"
#include "Config.h"
#include "MeshRenderObj.h"

// this is in bytes, should be about ~20mb, 650k 32 byte vertices
constexpr static uint32_t MAX_VERT_BUFF_SIZE = 650000 * 32;
// also in bytes for now just same size as vert buff
constexpr static uint32_t MAX_INDEX_BUFF_SIZE = MAX_VERT_BUFF_SIZE;

struct MeshConstants {
    glm::mat4 world;
};

MeshRenderer::~MeshRenderer() {
    // TODO: Cleanup renderObjs
    assert(false);
}

void MeshRenderer::OnInit() {

    gfx::BufferDesc vBufDesc = gfx::BufferDesc::vbPersistent(MAX_VERT_BUFF_SIZE, "MeshSharedVB");

    gfx::BufferDesc iBufDesc = gfx::BufferDesc::ibPersistent(MAX_INDEX_BUFF_SIZE, "MeshSharedIB");

    vertBufferId = device()->AllocateBuffer(vBufDesc);
    indexBufferId = device()->AllocateBuffer(iBufDesc);

    // whoops, this doesnt get deleted
    MeshRenderObj* renderObj = new MeshRenderObj("crytek/sponza.obj", "crytek/sponza.obj");
    Register(renderObj);
}

void MeshRenderer::Register(MeshRenderObj* meshObj) {
    meshObj->perObject = services()->constantBufferManager()->GetConstantBuffer(sizeof(MeshConstants), "MeshWorld");
    meshObj->mesh = services()->meshCache()->Get(meshObj->_meshName);
    meshObj->mat = services()->materialCache()->Get(meshObj->_matName);

    gfx::StateGroupEncoder encoder;
    encoder.Begin();
    encoder.BindResource(meshObj->perObject->GetBinding(1));
    encoder.SetVertexShader(services()->shaderCache()->Get(gfx::ShaderType::VertexShader, "blinn"));
    meshObj->stateGroup.reset(encoder.End());

    for (const MaterialData& matdata : meshObj->mat->matData) {
        std::string shaderName;
        switch (matdata.shadingModel) {
            // oh how convienient, their all blinn
        default:
            shaderName = "blinn";
        }
        gfx::ShaderId ps = services()->shaderCache()->Get(gfx::ShaderType::PixelShader, shaderName);
        meshObj->meshMaterial.push_back(std::make_unique<MeshMaterial>(device(), matdata, ps));
    }

    for (const MeshPart& part : meshObj->mesh->GetParts()) {
        assert(MAX_VERT_BUFF_SIZE > ((vertOffset + part.geometryData.vertexCount()) * part.geometryData.vertexLayout().stride()));
        assert(MAX_INDEX_BUFF_SIZE > ((indexOffset + part.geometryData.indexCount()) * sizeof(uint32_t)));

        MeshGeomExistBuffer existBuffer;
        existBuffer.indexBuffer = indexBufferId;
        existBuffer.indexOffset = indexOffset;
        existBuffer.vertexBuffer = vertBufferId;
        existBuffer.vertexOffset = vertOffset;

        meshObj->meshGeometry.push_back(std::make_unique<MeshGeometry>(device(), part.geometryData, &existBuffer));
        meshObj->meshGeometry.back()->meshMaterialId = part.matIdx;
        
        vertOffset += part.geometryData.vertexCount();
        indexOffset += part.geometryData.indexCount();
    }
    meshRenderObjs.push_back(meshObj);
}

void MeshRenderer::Submit(RenderQueue* renderQueue, const FrameView* renderView) {
    _drawItems.clear();
    sortedMatCache.clear();

    glm::mat4 world = glm::scale(glm::mat4(), glm::vec3(1, 1, 1));
    for (MeshRenderObj* renderObj : meshRenderObjs) {
        assert(renderObj->perObject);
        assert(renderObj->mat);
        assert(renderObj->mesh);

        MeshConstants* meshBuffer = renderObj->perObject->Map<MeshConstants>();
        meshBuffer->world = world;
        renderObj->perObject->Unmap();

        for (auto& mg : renderObj->meshGeometry) {
            gfx::DrawItemEncoder encoder;

            std::unique_ptr<const gfx::DrawItem> drawItem;

            uint32_t meshMatIdx = mg->meshMaterialId;
            assert(renderObj->meshMaterial.size() > meshMatIdx);

            std::vector<const gfx::StateGroup*> groups = { 
                renderObj->stateGroup.get(),
                mg->stateGroup(),
                renderObj->meshMaterial[meshMatIdx]->stateGroup(),
                renderQueue->defaults 
            };

            drawItem.reset(encoder.Encode(device(), mg->drawCall(), groups.data(), groups.size()));

            _drawItems.emplace_back(std::move(drawItem));
            sortedMatCache.emplace(meshMatIdx, _drawItems.back().get());
        }

        // cant find if iterator is guarenteed to go bucket by bucket, but still doing it live
        for (auto it = sortedMatCache.begin(); it != sortedMatCache.end(); ++it) {
            renderQueue->AddDrawItem(0, it->second);
        }
    }
}
