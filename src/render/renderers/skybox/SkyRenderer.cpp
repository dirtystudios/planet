#include "SkyRenderer.h"
#include <glm/gtx/transform.hpp>
#include "DGAssert.h"
#include "DrawItemEncoder.h"
#include "Image.h"
#include "MeshGeneration.h"
#include "Skybox.h"
#include "SkyboxVertex.h"
#include "StateGroupEncoder.h"

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
    encoder.SetVertexShader(services()->shaderCache()->Get(gfx::ShaderType::VertexShader, "sky"));
    encoder.SetPixelShader(services()->shaderCache()->Get(gfx::ShaderType::PixelShader, "sky"));
    encoder.SetVertexLayout(services()->vertexLayoutCache()->Get(SkyboxVertex::GetVertexLayoutDesc()));
    encoder.SetRasterState(rs);
    encoder.SetDepthState(ds);
    _base = encoder.End();
}

void SkyRenderer::Register(SkyboxRenderObj* skybox) {
    dg_assert_nm(skybox != nullptr);

    dimg::Image skyboxImages[6];
    for (uint32_t idx = 0; idx < 6; ++idx) {
        if (!LoadImageFromFile(skybox->_imagePaths[idx].c_str(), &skyboxImages[idx])) {
            LOG_E("Failed to load image: %s", skybox->_imagePaths[idx].c_str());
        }
    }

    void* datas[6] = {skyboxImages[0].data, skyboxImages[1].data, skyboxImages[2].data, skyboxImages[3].data, skyboxImages[4].data, skyboxImages[5].data};

    uint32_t       width           = skyboxImages[0].width;
    uint32_t       height          = skyboxImages[0].height;
    gfx::TextureId skyboxTextureId = device()->CreateTextureCube(gfx::PixelFormat::RGB8Unorm, width, height, datas);
    assert(skyboxTextureId);

    skybox->_textureCubeId = skyboxTextureId;

    uint32_t        vertexCount = 36;
    size_t          bufferSize  = sizeof(SkyboxVertex) * vertexCount;
    gfx::BufferDesc desc        = gfx::BufferDesc::defaultPersistent(gfx::BufferUsageFlags::VertexBufferBit, bufferSize);
    skybox->_vertexBuffer       = device()->AllocateBuffer(desc);

    glm::vec3* mapped = reinterpret_cast<glm::vec3*>(device()->MapMemory(skybox->_vertexBuffer, gfx::BufferAccess::Write));
    assert(mapped);

    uint32_t mappedVertices = 0;
    dgen::GenerateCube(
        [&](float x, float y, float z, float u, float v) {
            mapped->x = x;
            mapped->y = y;
            mapped->z = z;
            mapped++;
            mappedVertices++;
        },
        0.f, 0.f, 0.f);
    device()->UnmapMemory(skybox->_vertexBuffer);

    skybox->_constantBuffer = services()->constantBufferManager()->GetConstantBuffer(sizeof(SkyboxConstants));

    assert(vertexCount == mappedVertices);

    _objs.push_back(skybox);

    gfx::StateGroupEncoder encoder;
    encoder.Begin(_base);
    encoder.SetVertexBuffer(skybox->_vertexBuffer);
    encoder.BindResource(skybox->_constantBuffer->GetBinding(1));
    encoder.BindTexture(0, skybox->_textureCubeId);
    skybox->_group = encoder.End();
}

void SkyRenderer::Unregister(SkyboxRenderObj* renderObj) {
    // TODO: Actually remove it and cleanup
}

void SkyRenderer::Submit(RenderQueue* renderQueue, RenderView* renderView) {
    Camera*   camera    = renderView->camera;
    glm::mat4 translate = glm::translate(glm::mat4(), camera->pos);
    glm::mat4 scale     = glm::scale(glm::mat4(), glm::vec3(500.f, 500.f, 500.f));
    glm::mat4 model     = translate * scale;
    glm::mat4 world     = model;

    for (SkyboxRenderObj* skybox : _objs) {
        if (skybox->_item == nullptr) {
            gfx::DrawCall drawCall;
            drawCall.type           = gfx::DrawCall::Type::Arrays;
            drawCall.offset         = 0;
            drawCall.primitiveCount = 36;

            std::vector<const gfx::StateGroup*> groups = {skybox->_group, renderQueue->defaults};
            skybox->_item                              = gfx::DrawItemEncoder::Encode(device(), drawCall, groups.data(), groups.size());
        }

        skybox->_constantBuffer->Map<SkyboxConstants>()->world = world;
        skybox->_constantBuffer->Unmap();

        renderQueue->AddDrawItem(0, skybox->_item);
    }
}
