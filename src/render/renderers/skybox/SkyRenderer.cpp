#include "SkyRenderer.h"
#include "MeshGeneration.h"
#include "Image.h"
#include "Skybox.h"
#include "SkyboxVertex.h"
#include <glm/gtx/transform.hpp>
#include "StateGroupEncoder.h"
#include "DrawItemEncoder.h"

struct SkyboxConstants {
    glm::mat4 world;
};

SkyRenderer::~SkyRenderer() {
    // TODO: Cleanup renderobjs
}

void SkyRenderer::OnInit() {
    gfx::RasterState rs;
    rs.cullMode = gfx::CullMode::None;

    gfx::DepthState ds;
    ds.depthFunc = gfx::DepthFunc::LessEqual;
    ds.enable    = true;

    gfx::StateGroupEncoder encoder;
    encoder.Begin();
    encoder.SetVertexShader(GetShaderCache()->Get(gfx::ShaderType::VertexShader, "sky"));
    encoder.SetPixelShader(GetShaderCache()->Get(gfx::ShaderType::PixelShader, "sky"));
    encoder.SetVertexLayout(GetVertexLayoutCache()->Get(SkyboxVertex::GetVertexLayoutDesc()));
    encoder.SetRasterState(rs);
    encoder.SetDepthState(ds);
    _base = encoder.End();
}

RenderObj* SkyRenderer::Register(SimObj* simObj) {
    assert(simObj);
    // TODO: simobj should have association with a camera
    assert(simObj->HasComponents({ComponentType::Skybox}));
    Skybox* skyBox = simObj->GetComponent<Skybox>(ComponentType::Skybox);

    Image skyboxImages[6];
    for (uint32_t idx = 0; idx < 6; ++idx) {
        if (!LoadImageFromFile(skyBox->imagePaths[idx].c_str(), &skyboxImages[idx])) {
            LOG_E("Failed to load image: %s", skyBox->imagePaths[idx].c_str());
        }
    }

    void* datas[6] = {skyboxImages[0].data, skyboxImages[1].data, skyboxImages[2].data,
                      skyboxImages[3].data, skyboxImages[4].data, skyboxImages[5].data};

    uint32_t width  = skyboxImages[0].width;
    uint32_t height = skyboxImages[0].height;
    gfx::TextureId skyboxTextureId =
        GetRenderDevice()->CreateTextureCube(gfx::TextureFormat::RGB_U8, width, height, datas);
    assert(skyboxTextureId);

    SkyboxRenderObj* renderObj = new SkyboxRenderObj();
    renderObj->textureCubeId   = skyboxTextureId;

    uint32_t vertexCount    = 36;
    size_t bufferSize       = sizeof(SkyboxVertex) * vertexCount;
    gfx::BufferDesc desc    = gfx::BufferDesc::defaultPersistent(gfx::BufferUsageFlags::VertexBufferBit, bufferSize);
    renderObj->vertexBuffer = GetRenderDevice()->AllocateBuffer(desc);

    glm::vec3* mapped =
        reinterpret_cast<glm::vec3*>(GetRenderDevice()->MapMemory(renderObj->vertexBuffer, gfx::BufferAccess::Write));
    assert(mapped);

    uint32_t mappedVertices = 0;
    meshGen::GenerateCube([&](float x, float y, float z, float u, float v) {
        mapped->x = x;
        mapped->y = y;
        mapped->z = z;
        mapped++;
        mappedVertices++;
    }, 0.f, 0.f, 0.f);
    GetRenderDevice()->UnmapMemory(renderObj->vertexBuffer);

    renderObj->constantBuffer = GetConstantBufferManager()->GetConstantBuffer(sizeof(SkyboxConstants));

    assert(vertexCount == mappedVertices);

    _objs.push_back(renderObj);

    gfx::StateGroupEncoder encoder;
    encoder.Begin(_base);
    encoder.SetVertexBuffer(renderObj->vertexBuffer);
    encoder.BindResource(renderObj->constantBuffer->GetBinding(1));
    encoder.BindTexture(0, renderObj->textureCubeId);
    renderObj->group = encoder.End();

    gfx::DrawCall drawCall;
    drawCall.type           = gfx::DrawCall::Type::Arrays;
    drawCall.offset         = 0;
    drawCall.primitiveCount = vertexCount;

    renderObj->item = gfx::DrawItemEncoder::Encode(GetRenderDevice(), drawCall, &renderObj->group, 1);

    return renderObj;
}
void SkyRenderer::Unregister(RenderObj* renderObj) {
    assert(renderObj->GetRendererType() == RendererType::Skybox);
    // TODO: Actually remove it and cleanup
}

void SkyRenderer::Submit(RenderQueue* renderQueue, RenderView* renderView) {
    Camera* camera      = renderView->camera;
    glm::mat4 translate = glm::translate(glm::mat4(), camera->pos);
    glm::mat4 scale     = glm::scale(glm::mat4(), glm::vec3(500.f, 500.f, 500.f));
    glm::mat4 model     = translate * scale;
    glm::mat4 world     = model;

    for (SkyboxRenderObj* skybox : _objs) {
        skybox->constantBuffer->Map<SkyboxConstants>()->world = world;
        skybox->constantBuffer->Unmap();

        renderQueue->AddDrawItem(0, skybox->item);
    }
}
