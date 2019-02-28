#include "RayTraceRenderer.h"
#include "Log.h"
#include <glm/gtx/transform.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/norm.hpp>
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
#include "Image.h"

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

struct DirLight {
    glm::vec3 direction;
    float intensity;
};

struct RTPerObject {
    glm::vec2 pixOffsets;
    glm::vec2 p1;
    DirLight dirLight;
};

struct RTSphere {
    glm::vec3 pos;
    float radius;
    glm::vec3 albedo;
    glm::vec3 specular;
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

    std::string assetDirPath = config::Config::getInstance().GetConfigString("RenderDeviceSettings", "AssetDirectory");
    if (!fs::IsPathDirectory(assetDirPath)) {
        LOG_E("Invalid Directory Path given for AssetDirectory.");
    }

    std::string imagepath = assetDirPath + "/skyboxcape/cape_hill_1k.hdr";

    dimg::Image skyboxImage;
    if (!dimg::LoadImageFromFile(imagepath.c_str(), &skyboxImage)) {
        LOG_E("Failed to load image: %s", imagepath.c_str());
    }

    uint32_t width = skyboxImage.width;
    uint32_t height = skyboxImage.height;
    skyboxTextureId = device()->CreateTexture2D(PixelFormat::RGB8Unorm, TextureUsageFlags::ShaderRead, width, height, skyboxImage.data, "rayTraceSkybox");
    assert(skyboxTextureId);

    csVertBuffer = device()->AllocateBuffer(gfx::BufferDesc::defaultPersistent(gfx::BufferUsageFlags::ShaderBufferBit, kDefaultVertexBufferSize, "rayTraceVB"));
    fakeVertBuff = device()->AllocateBuffer(gfx::BufferDesc::defaultPersistent(gfx::BufferUsageFlags::VertexBufferBit, kDefaultVertexBufferSize, "rayTraceFakeVB"));
    cbPerObj = services()->constantBufferManager()->GetConstantBuffer(sizeof(RTPerObject), "rayTracePerObj");
    rpCbPerObj = services()->constantBufferManager()->GetConstantBuffer(sizeof(unsigned int), "rayTraceAAPerObj");

    SetupSpheres();
    assert(sphereBuff != NULL_ID);
    
    computeResultTex = device()->CreateTexture2D(PixelFormat::RGBA32Float, TextureUsageFlags::ShaderRW, 1920, 1080, nullptr, "rayTraceImmResultTex");

    gfx::StateGroupEncoder encoder;

    encoder.Begin();
    encoder.BindBuffer(0, csVertBuffer, ShaderStageFlags::ComputeBit);
    encoder.BindBuffer(2, sphereBuff, ShaderStageFlags::ComputeBit);
    encoder.BindResource(cbPerObj->GetBinding(2));
    encoder.BindTexture(0, computeResultTex, ShaderStageFlags::ComputeBit, ShaderBindingFlags::ReadWrite);
    encoder.BindTexture(1, skyboxTextureId, ShaderStageFlags::ComputeBit);
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
    encoder.BindTexture(0, computeResultTex, ShaderStageFlags::PixelBit, ShaderBindingFlags::SampleRead);
    encoder.BindResource(rpCbPerObj->GetBinding(2));
    encoder.SetVertexShader(services()->shaderCache()->Get(gfx::ShaderType::VertexShader, "FSQuadAA"));
    encoder.SetVertexLayout(services()->vertexLayoutCache()->Pos3f());
    encoder.SetVertexBuffer(fakeVertBuff);
    encoder.SetPixelShader(services()->shaderCache()->Get(gfx::ShaderType::PixelShader, "FSQuadAA"));
    encoder.SetBlendState(blendState);
    encoder.SetRasterState(rs);
    encoder.SetDepthState(ds);
    renderStateGroup = encoder.End();
}

void RayTraceRenderer::SetupSpheres() {
    auto sphereRadius = glm::vec2(3.f, 10.f);
    int spheresmax = 400;
    float placementRadius = 80.f;

    std::vector<RTSphere> spheres;
    spheres.reserve(spheresmax);

    for (int i = 0; i < spheresmax; ++i) {
        RTSphere s;
        s.radius = sphereRadius.x + glm::linearRand(0.f, 1.f) * (sphereRadius.y - sphereRadius.x);
        glm::vec2 randPos = glm::sphericalRand(1.f) * placementRadius;
        s.pos = glm::vec3(randPos.x, s.radius, randPos.y);

        // reject spheres that intersec tothers
        bool reject = false;
        for (auto o : spheres) {
            float minDist = s.radius + o.radius;
            if (glm::length2(s.pos - o.pos) < minDist * minDist) {
                reject = true;
                break;
            }
        }

        if (reject) continue;

        glm::vec4 color = glm::linearRand(glm::vec4(0.f), glm::vec4(1.f));
        bool metal = glm::linearRand(0.f, 1.f) < 0.5f;
        s.albedo = metal ? glm::vec3(0.f) : glm::vec3(color.r, color.g, color.b);
        s.specular = metal ? glm::vec3(color.r, color.g, color.b) : glm::vec3(1.f) * 0.04f;
        spheres.push_back(std::move(s));
    }

    auto desc = gfx::BufferDesc::defaultPersistent(gfx::BufferUsageFlags::ShaderBufferBit, sizeof(RTSphere) * spheresmax, "rayTraceSpheres");
    desc.stride = sizeof(RTSphere);
    sphereBuff = device()->AllocateBuffer(desc, spheres.data());
}

void RayTraceRenderer::Register(MeshRenderObj* meshObj) {
}

void RayTraceRenderer::Submit(RenderQueue* renderQueue, const FrameView* renderView) {
    _drawItems.clear();

    bool hasChanged = !(*renderView == *lastFrameView);
    if (hasChanged) {
        currentSample = 0;
        lastFrameView = std::make_unique<FrameView>(*renderView);
    }
    else
        currentSample++;

    auto perObj = rpCbPerObj->Map<unsigned int>();
    *perObj = currentSample;
    rpCbPerObj->Unmap();

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

void RayTraceRenderer::Submit(ComputeQueue* cQueue, const FrameView* renderView) {
    assert(cbPerObj);

    _dispatchItems.clear();

    auto perObj = cbPerObj->Map<RTPerObject>();
    perObj->pixOffsets = glm::vec2(glm::linearRand(0.f, 1.f), glm::linearRand(0.f, 1.f));
    perObj->dirLight.direction = glm::vec3(0.5f, -1.f, 0.f);
    perObj->dirLight.intensity = 0.8f;
    cbPerObj->Unmap();

    std::vector<const gfx::StateGroup*> groups = {
        csStateGroup,
        cQueue->defaults
    };

    DispatchCall dc;
    dc.groupX = 1920 / 8;//renderView->viewport.width / 8;
    dc.groupY = 1080 / 8;// renderView->viewport.height / 8;
    dc.groupZ = 1;

    DispatchItemEncoder die;
    std::unique_ptr<const gfx::DispatchItem> di;

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
