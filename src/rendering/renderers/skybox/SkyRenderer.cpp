#include "SkyRenderer.h"
#include "MeshGeneration.h"
#include "Image.h"
#include "Skybox.h"
#include "SkyboxVertex.h"
#include <glm/gtx/transform.hpp>

struct SkyboxConstants {
    glm::mat4 world;
};

SkyRenderer::~SkyRenderer() {
    // TODO: Cleanup renderobjs
}

void SkyRenderer::OnInit() {
    graphics::PipelineStateDesc psd;
    psd.vertexShader         = GetShaderCache()->Get(graphics::ShaderType::VertexShader, "sky");
    psd.pixelShader          = GetShaderCache()->Get(graphics::ShaderType::PixelShader, "sky");
    psd.vertexLayout         = GetVertexLayoutCache()->Get(SkyboxVertex::GetVertexLayoutDesc());
    psd.topology             = graphics::PrimitiveType::Triangles;
    psd.rasterState.cullMode = graphics::CullMode::None;
    psd.depthState.depthFunc = graphics::DepthFunc::LessEqual;
    psd.depthState.enable    = true;
    _defaultPS = GetPipelineStateCache()->Get(psd);
    assert(_defaultPS);
}

RenderObj* SkyRenderer::Register(SimObj* simObj) {
    assert(simObj);
    // TODO: simobj should have association with a camera
    assert(simObj->HasComponents({ComponentType::Skybox}));
    Skybox* skyBox = simObj->GetComponent<Skybox>(ComponentType::Skybox);

    Image skyboxImages[6] = {0};
    for (uint32_t idx = 0; idx < 6; ++idx) {
        if (!LoadImageFromFile(skyBox->imagePaths[idx].c_str(), &skyboxImages[idx])) {
            LOG_E("Failed to load image: %s", skyBox->imagePaths[idx].c_str());
        }
    }

    void* datas[6] = {skyboxImages[0].data, skyboxImages[1].data, skyboxImages[2].data,
                      skyboxImages[3].data, skyboxImages[4].data, skyboxImages[5].data};

    uint32_t width  = skyboxImages[0].width;
    uint32_t height = skyboxImages[0].height;
    graphics::TextureId skyboxTextureId =
        GetRenderDevice()->CreateTextureCube(graphics::TextureFormat::RGB_U8, width, height, datas);
    assert(skyboxTextureId);

    SkyboxRenderObj* renderObj = new SkyboxRenderObj();
    renderObj->textureCubeId   = skyboxTextureId;
    renderObj->vertexCount     = 36;

    size_t bufferSize       = sizeof(SkyboxVertex) * renderObj->vertexCount;
    renderObj->vertexBuffer = GetRenderDevice()->AllocateBuffer(graphics::BufferType::VertexBuffer, bufferSize,
                                                                graphics::BufferUsage::Static);

    glm::vec3* mapped = reinterpret_cast<glm::vec3*>(
        GetRenderDevice()->MapMemory(renderObj->vertexBuffer, graphics::BufferAccess::Write));
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

    assert(renderObj->vertexCount == mappedVertices);

    _objs.push_back(renderObj);

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
        graphics::DrawItemDesc did;
        did.drawCall.type           = graphics::DrawCall::Type::Arrays;
        did.drawCall.offset         = 0;
        did.drawCall.primitiveCount = 36;
        did.pipelineState           = _defaultPS;
        did.streamCount             = 1;
        did.streams[0].vertexBuffer = skybox->vertexBuffer;
        did.streams[0].offset       = 0;
        did.streams[0].stride       = sizeof(glm::vec3);
        did.bindingCount            = 2;
        did.bindings[0].type        = graphics::Binding::Type::Texture;
        did.bindings[0].slot        = 0;
        did.bindings[0].resource    = skybox->textureCubeId;
        did.bindings[1]             = skybox->constantBuffer->GetBinding(1);

        const graphics::DrawItem* item = GetRenderDevice()->GetDrawItemEncoder()->Encode(did);
        renderQueue->AddDrawItem(0, item);
    }
}
