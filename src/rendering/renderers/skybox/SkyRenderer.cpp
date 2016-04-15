#include "SkyRenderer.h"
#include "MeshGeneration.h"
#include "Image.h"
#include "Skybox.h"
#include "SkyboxVertex.h"
#include <glm/gtx/transform.hpp>

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
    _defaultPS               = GetPipelineStateCache()->Get(psd);
    _transform = GetRenderDevice()->CreateShaderParam(psd.vertexShader, "world", graphics::ParamType::Float4x4);
    assert(_defaultPS);
    assert(_transform);
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

    renderObj->vertexCount = 36;
    std::vector<SkyboxVertex> vertices;
    vertices.reserve(renderObj->vertexCount);

    meshGen::VertexDelegate delegate = [&vertices](float x, float y, float z, float u, float v) {
        vertices.push_back({glm::vec3(x, y, z)});
    };

    meshGen::GenerateCube(delegate, 0.f, 0.f, 0.f);
    assert(renderObj->vertexCount == vertices.size());
    renderObj->vertexBuffer =
        GetRenderDevice()->CreateBuffer(graphics::BufferType::VertexBuffer, vertices.data(),
                                        sizeof(SkyboxVertex) * vertices.size(), graphics::BufferUsage::Static);

    _objs.push_back(renderObj);

    return renderObj;
}
void SkyRenderer::Unregister(RenderObj* renderObj) {
    assert(renderObj->GetRendererType() == RendererType::Skybox);
    // TODO: Actually remove it and cleanup
}

void SkyRenderer::Submit(RenderQueue* renderQueue, RenderView* renderView) {
    Camera* camera      = renderView->camera;
    glm::mat4 perspView = camera->BuildProjection() * camera->BuildView();
    glm::mat4 translate = glm::translate(glm::mat4(), camera->pos);
    glm::mat4 scale     = glm::scale(glm::mat4(), glm::vec3(500.f, 500.f, 500.f));
    glm::mat4 model     = translate * scale;
    glm::mat4 world     = perspView * model;

    for (SkyboxRenderObj* skybox : _objs) {
        graphics::DrawTask* task = renderQueue->AppendTask(0);
        task->BindTexture(graphics::ShaderStage::Pixel, skybox->textureCubeId, graphics::TextureSlot::Base);
        task->UpdateShaderParam(_transform, &world);

        task->pipelineState = _defaultPS;
        task->vertexBuffer  = skybox->vertexBuffer;
        task->vertexCount   = skybox->vertexCount;
        task->vertexOffset  = skybox->vertexOffset;
    }
}
