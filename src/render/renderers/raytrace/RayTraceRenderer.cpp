#include "RayTraceRenderer.h"
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
#include "MeshGeneration.h"

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

    gfx::BufferDesc desc = gfx::BufferDesc::defaultPersistent(gfx::BufferUsageFlags::ShaderBufferBit, kDefaultVertexBufferSize, "rayTraceVB");
    vertBuffer = device()->AllocateBuffer(desc);

    resultTex = device()->CreateTexture2D(PixelFormat::RGBA32Float, TextureUsageFlags::ShaderRW, 1280, 800, nullptr, "rayTraceResultTex");

    gfx::StateGroupEncoder encoder;

    encoder.Begin();
    encoder.BindBuffer(0, vertBuffer, ShaderStageFlags::ComputeBit);
    encoder.BindTexture(0, resultTex, ShaderStageFlags::ComputeBit);
    encoder.SetComputeShader(services()->shaderCache()->Get(gfx::ShaderType::ComputeShader, "rayTraceKernel"));

    stateGroup = encoder.End();
}

void RayTraceRenderer::Register(MeshRenderObj* meshObj) {
    /*meshObj->perObject = services()->constantBufferManager()->GetConstantBuffer(sizeof(MeshConstants), "MeshWorld");
    meshObj->lighting = services()->constantBufferManager()->GetConstantBuffer(sizeof(Lighting), "Lighting");


    gfx::RasterState rs;
    rs.cullMode = gfx::CullMode::None;  
    rs.windingOrder = gfx::WindingOrder::FrontCW;

    gfx::BlendState blendState;
    blendState.enable = true;
    blendState.srcRgbFunc = gfx::BlendFunc::SrcAlpha;
    blendState.srcAlphaFunc = blendState.srcRgbFunc;
    blendState.dstRgbFunc = gfx::BlendFunc::OneMinusSrcAlpha;
    blendState.dstAlphaFunc = blendState.dstRgbFunc;
    
    gfx::StateGroupEncoder encoder;
    encoder.Begin();
    encoder.BindResource(meshObj->perObject->GetBinding(1));
    encoder.BindResource(meshObj->lighting->GetBinding(3));
    encoder.SetVertexShader(services()->shaderCache()->Get(gfx::ShaderType::VertexShader, "blinn"));
    //encoder.SetRasterState(rs);
    encoder.SetBlendState(blendState);
    meshObj->stateGroup.reset(encoder.End());

    for (const MaterialData& matdata : meshObj->mat->matData) {
        std::string shaderName;
        switch (matdata.shadingModel) {
            // oh how convienient, they're all blinn
        default:
            shaderName = "blinn";
        }
        gfx::ShaderId ps = services()->shaderCache()->Get(gfx::ShaderType::PixelShader, shaderName);
        meshObj->meshMaterial.push_back(std::make_unique<MeshMaterial>(device(), matdata, ps));
    }

    for (const auto& boneInfo : meshObj->mesh->GetBones()) {
        meshObj->_boneOffsets.emplace_back(boneInfo.second);
    }

    meshRenderObjs.push_back(meshObj);*/
}

void RayTraceRenderer::Submit(RenderQueue* renderQueue, const FrameView* renderView) {
    
}

void RayTraceRenderer::Submit(ComputeQueue* cQueue) {

    DirLight dirLight{};
    dirLight.direction = glm::vec3(-0.2f, -0.3f, -1.f);
    dirLight.ambient = glm::vec3(0.5f, 0.5f, 0.5f);
    dirLight.diffuse = glm::vec3(0.9f, 0.9f, 0.9f);
    dirLight.specular = glm::vec3(0.3f, 0.3f, 0.3f);
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
