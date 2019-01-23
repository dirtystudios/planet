#include "RayTraceRenderer.h"
#include "Log.h"
#include <glm/gtx/transform.hpp>
#include "ConstantBuffer.h"
#include "ConstantBufferManager.h"
#include "StateGroupEncoder.h"
#include "MeshGeometry.h"
#include "DrawItemEncoder.h"
#include "DispatchItemEncoder.h"
#include "Image.h"
#include "Config.h"
#include "MeshRenderObj.h"
#include "MeshGeneration.h"

#include <memory>

struct MeshVertex {
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec3 color;
};

constexpr size_t kDefaultVertexBufferSize = sizeof(MeshVertex) * 4096;

struct MeshConstants {
    glm::mat4 world;
    std::array<glm::mat4, 255> boneOffsets;
};

struct PointLight {
    glm::vec3 pos;
    float p1;

    float constant;
    float linear;
    float quadratic;
    float p2;

    glm::vec3 ambient;
    float p3;
    glm::vec3 diffuse;
    float p4;
    glm::vec3 specular;
    float p5;
};

struct DirLight {
    glm::vec3 direction;
    float p1;

    glm::vec3 ambient;
    float p2;
    glm::vec3 diffuse;
    float p3;
    glm::vec3 specular;
    float p4;
};

struct Lighting {
    PointLight pointLights[4];
    DirLight dirLight;
    int numPointLights;
    glm::vec3 p1;
};

RayTraceRenderer::~RayTraceRenderer() {
    // TODO: Cleanup renderObjs
    assert(false);
}

void RayTraceRenderer::OnInit() {
    MeshGeometryData geometryData;
    dgen::GenerateIcoSphere(5, &geometryData);
    sphereGeom.reset(new MeshGeometry(device(), { geometryData }));

    csVertBuffer = device()->AllocateBuffer(gfx::BufferDesc::defaultPersistent(gfx::BufferUsageFlags::ShaderBufferBit, kDefaultVertexBufferSize, "rayTraceVB"));
    fakeVertBuff = device()->AllocateBuffer(gfx::BufferDesc::defaultPersistent(gfx::BufferUsageFlags::VertexBufferBit, kDefaultVertexBufferSize, "rayTraceFakeVB"));

    resultTex = device()->CreateTexture2D(PixelFormat::RGBA32Float, TextureUsageFlags::ShaderRW, 1280, 720, nullptr, "rayTraceResultTex");

    gfx::StateGroupEncoder encoder;

    encoder.Begin();
    encoder.BindBuffer(0, csVertBuffer, ShaderStageFlags::ComputeBit);
    encoder.BindTexture(0, resultTex, ShaderStageFlags::ComputeBit);
    encoder.SetComputeShader(services()->shaderCache()->Get(gfx::ShaderType::ComputeShader, "rayTraceKernel"));
    csStateGroup = encoder.End();

    gfx::RasterState rs;
    rs.cullMode = gfx::CullMode::None;

    gfx::BlendState blendState;
    blendState.enable = true;
    blendState.srcRgbFunc = gfx::BlendFunc::SrcAlpha;
    blendState.srcAlphaFunc = blendState.srcRgbFunc;
    blendState.dstRgbFunc = gfx::BlendFunc::OneMinusSrcAlpha;
    blendState.dstAlphaFunc = blendState.dstRgbFunc;

    gfx::DepthState ds;
    ds.enable = false;

    encoder.Begin();
    encoder.BindTexture(0, resultTex, ShaderStageFlags::PixelBit);
    encoder.SetVertexShader(services()->shaderCache()->Get(gfx::ShaderType::VertexShader, "FSQuad"));
    encoder.SetVertexLayout(services()->vertexLayoutCache()->Pos3f());
    encoder.SetVertexBuffer(fakeVertBuff);
    encoder.SetPixelShader(services()->shaderCache()->Get(gfx::ShaderType::PixelShader, "FSQuad"));
    encoder.SetBlendState(blendState);
    encoder.SetRasterState(rs);
    encoder.SetDepthState(ds);
    renderStateGroup = encoder.End();
}

void RayTraceRenderer::Register(MeshRenderObj* meshObj) {
}

void RayTraceRenderer::Submit(RenderQueue* renderQueue, const FrameView* renderView) {
    _drawItems.clear();

    gfx::DrawCall drawCall;
    drawCall.type = gfx::DrawCall::Type::Arrays;
    drawCall.startOffset = 0;
    drawCall.primitiveCount = 3;

    std::vector<const gfx::StateGroup*> groups = {
        renderStateGroup,
        renderQueue->defaults
    };

    _drawItems.emplace_back(DrawItemEncoder::Encode(device(), drawCall, groups.data(), groups.size()));

    renderQueue->AddDrawItem(0, _drawItems.back().get());
}

void RayTraceRenderer::Submit(ComputeQueue* cQueue) {
    _dispatchItems.clear();

    DirLight dirLight{};
    dirLight.direction = glm::vec3(-0.2f, -0.3f, -1.f);
    dirLight.ambient = glm::vec3(0.5f, 0.5f, 0.5f);
    dirLight.diffuse = glm::vec3(0.9f, 0.9f, 0.9f);
    dirLight.specular = glm::vec3(0.3f, 0.3f, 0.3f);

    DispatchItemEncoder die;
    std::unique_ptr<const gfx::DispatchItem> di;

    std::vector<const gfx::StateGroup*> groups = {
        csStateGroup,
        cQueue->defaults
    };

    DispatchCall dc;
    dc.groupX = 1280 / 8;
    dc.groupY = 720 / 8;
    dc.groupZ = 1;

    di.reset(die.Encode(device(), dc, groups.data(), groups.size()));

    _dispatchItems.emplace_back(std::move(di));

    cQueue->AddDispatchItem(0, _dispatchItems.back().get());

    /*
    // todo: switch this back to renderview
    for (RenderObj* baseRO : meshRenderObjs) {
        if (baseRO->GetRendererType() != Renderer::rendererType()) {
            continue;
        }
        MeshRenderObj* renderObj = static_cast<MeshRenderObj*>(baseRO);

        glm::mat4 world = renderObj->_transform.matrix();

        assert(renderObj->perObject);
        assert(renderObj->mat);
        assert(renderObj->mesh);
        assert(renderObj->lighting);

        MeshConstants* meshBuffer = renderObj->perObject->Map<MeshConstants>();
        assert(renderObj->_boneOffsets.size() <= meshBuffer->boneOffsets.size());

        std::memcpy(meshBuffer->boneOffsets.data(), renderObj->_boneOffsets.data(), std::min(renderObj->_boneOffsets.size(), meshBuffer->boneOffsets.size()) * sizeof(glm::mat4));
        meshBuffer->world = world;
        renderObj->perObject->Unmap();

        auto* lighting = renderObj->lighting->Map<Lighting>();
        std::memcpy(&lighting->dirLight, &dirLight, sizeof(DirLight));
        lighting->numPointLights = 0;
        renderObj->lighting->Unmap();

        for (const auto& mg : renderObj->mesh->GetMeshGeometry()) {
            gfx::DrawItemEncoder encoder;

            std::unique_ptr<const gfx::DrawItem> drawItem;

            uint32_t meshMatIdx = mg.meshMaterialId;
            assert(renderObj->meshMaterial.size() > meshMatIdx);

            std::vector<const gfx::StateGroup*> groups = {
                renderObj->stateGroup.get(),
                mg.stateGroup(),
                renderObj->meshMaterial[meshMatIdx]->stateGroup(),
                cQueue->defaults
            };

            drawItem.reset(encoder.Encode(device(), mg.drawCall(), groups.data(), groups.size()));
        }
    }*/
}
