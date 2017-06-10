#include "RenderEngine.h"
#include <cassert>
#include "Config.h"
#include "ConstantBuffer.h"
#include "ConstantBuffer.h"
#include "DebugDrawInterface.h"
#include "DebugRenderer.h"
#include "DebugRenderer.h"
#include "MeshRenderer.h"
#include "SkyRenderer.h"
#include "StateGroupEncoder.h"
#include "TerrainRenderer.h"
#include "TextRenderer.h"
#include "UIRenderer.h"

using namespace gfx;

struct ViewConstants {
    glm::vec3 eye;
    float     padding0;
    glm::mat4 view;
    glm::mat4 proj;
};

// Todo::Where should view constantbuffer be...renderview?
using ViewConstantsBuffer = TypedConstantBuffer<ViewConstants>;
ConstantBuffer* viewConstantsBuffer;

CommandBuffer* cmdbuf;


RenderPass pass;

std::array<Renderer*, 6> Renderers::all() {
    return { {
        ui.get(),
        text.get(),
        debug.get(),
        terrain.get(),
        sky.get(),
        mesh.get()
    } };
}

RenderEngine::RenderEngine(RenderDevice* device, RenderView* view) : _device(device), _view(view) {
    _renderers.sky.reset(new SkyRenderer());
    _renderers.text.reset(new TextRenderer());
    _renderers.ui.reset(new UIRenderer());
    _renderers.mesh.reset(new MeshRenderer());
    _renderers.debug.reset(new DebugRenderer());
    _renderers.terrain.reset(new TerrainRenderer());

    _renderersByType.insert({RendererType::Skybox, _renderers.sky.get()});
    _renderersByType.insert({RendererType::Mesh, _renderers.mesh.get()});
    _renderersByType.insert({RendererType::Ui, _renderers.ui.get()});
    _renderersByType.insert({RendererType::Text, _renderers.text.get()});
    _renderersByType.insert({RendererType::Debug, _renderers.debug.get()});
    _renderersByType.insert({RendererType::Terrain, _renderers.terrain.get()});
    
    for (Renderer* renderer : _renderers.all()) {
        for (RenderPassType passType : renderer->supportedPasses()) {
            dg_assert_nm(passType != RenderPassType::Invalid);
            _renderersByPass[passType].push_back(renderer);  
        }        
    }

    _renderers.mesh->SetActive(true);
    _renderers.sky->SetActive(false);
    _renderers.terrain->SetActive(true);
    _renderers.text->SetActive(false);
    _renderers.ui->SetActive(false);

    std::string shaderDirPath = config::Config::getInstance().GetConfigString("RenderDeviceSettings", "ShaderDirectory");
    std::string assetDirPath  = config::Config::getInstance().GetConfigString("RenderDeviceSettings", "AssetDirectory");

    _shaderCache           = new ShaderCache(_device, shaderDirPath);
    _pipelineStateCache    = new PipelineStateCache(_device);
    _vertexLayoutCache     = new VertexLayoutCache(_device);
    _meshCache             = new MeshCache(_device, assetDirPath);
    _constantBufferManager = new ConstantBufferManager(_device);
    _materialCache         = new MaterialCache(_device, assetDirPath);
    _animationCache        = new AnimationCache(_device, assetDirPath);

    viewConstantsBuffer = _constantBufferManager->GetConstantBuffer(sizeof(ViewConstants), "ViewConstants");
    cmdbuf              = _device->CreateCommandBuffer();
    for (Renderer* renderer : _renderers.all()) {
        LOG_D("Initializing Renderer: %d", renderer->rendererType());
        renderer->Init(_device, this);
    }

    StateGroupEncoder encoder;
    encoder.Begin();
    encoder.SetBlendState(BlendState());
    encoder.SetRasterState(RasterState());
    encoder.SetDepthState(DepthState());
    encoder.BindResource(viewConstantsBuffer->GetBinding(0));
    _stateGroupDefaults = encoder.End();
    
    SwapChainInfo swapChainInfo = _device->GetSwapChainInfo();
    
    pass.colorAttachmentCount = 1;
    pass.colorAttachments[0] = _device->BackBufferAttachment();
    pass.depthAttachment = _device->CreateAttachment(gfx::PixelFormat::Depth32Float, swapChainInfo.width, swapChainInfo.height);
    
    RenderPassDesc passDesc;
    passDesc.colorAttachments = { pass.colorAttachments[0]};
    passDesc.depthAttachment = pass.depthAttachment;
    
    pass.type = RenderPassType::Standard;
    pass.passId = _device->CreateRenderPass(passDesc);
    
//    TextureId gbufferPosition = _device->CreateAttachment(gfx::PixelFormat::RGBA32Float, swapChainInfo.width, swapChainInfo.height);
//    TextureId gbufferAlbedo = _device->CreateAttachment(gfx::PixelFormat::RGBA32Float, swapChainInfo.width, swapChainInfo.height);
//    TextureId gbufferNormal = _device->CreateAttachment(gfx::PixelFormat::RGBA32Float, swapChainInfo.width, swapChainInfo.height);
//    TextureId depthBuffer = _device->CreateAttachment(gfx::PixelFormat::Depth32Float, swapChainInfo.width, swapChainInfo.height);
//
//    RenderPassDesc gbufferPassDesc;
//    gbufferPassDesc.colorAttachments = { gbufferPosition, gbufferAlbedo, gbufferNormal };
//    gbufferPassDesc.depthAttachment = depthBuffer;
//
//    RenderPassDesc compositionPassDesc;
//    compositionPassDesc.inputAttachments = { gbufferPosition, gbufferAlbedo, gbufferNormal };
//    compositionPassDesc.colorAttachments = { RenderDevice::BackBufferAttachment() };
//    compositionPassDesc.depthAttachment = depthBuffer;
//
//    RenderPassId gbufferPass = _device->CreateRenderPass(gbufferPassDesc);
//    RenderPassId compositionPass = _device->CreateRenderPass(compositionPassDesc);
}

RenderEngine::~RenderEngine() {
    if (_shaderCache) {
        delete _shaderCache;
        _shaderCache = nullptr;
    }
    if (_pipelineStateCache) {
        delete _pipelineStateCache;
        _pipelineStateCache = nullptr;
    }

    _renderersByType.clear();

    if (_stateGroupDefaults)
        delete _stateGroupDefaults;
}

void RenderEngine::RenderFrame(const RenderScene* scene) {
    cmdbuf->Reset();
    RenderQueue queue(cmdbuf);
    queue.defaults = _stateGroupDefaults;

    std::vector<RenderPassType> passesForFrame = { RenderPassType::Standard }; 

    assert(_view);
    FrameView view = _view->frameView();
    view._visibleObjects = scene->renderObjects; // for now
    
    // update view constants
    ViewConstants* mapped = viewConstantsBuffer->Map<ViewConstants>();
    mapped->eye           = view.eyePos;
    mapped->proj          = view.projection;
    mapped->view          = view.view;
    viewConstantsBuffer->Unmap();

    for (RenderPassType renderPassType : passesForFrame) {
        dg_assert_nm(renderPassType != RenderPassType::Invalid);
        const std::vector<Renderer*>& renderers = _renderersByPass[renderPassType];
        for (Renderer* renderer : renderers) {
            if (renderer->isActive()) {
                renderer->Submit(&queue, &view);
            }    
        }
    }
    
    queue.Submit(_device);
    _device->RenderFrame();
}

ShaderCache*           RenderEngine::shaderCache() { return _shaderCache; }
PipelineStateCache*    RenderEngine::pipelineStateCache() { return _pipelineStateCache; }
VertexLayoutCache*     RenderEngine::vertexLayoutCache() { return _vertexLayoutCache; }
MeshCache*             RenderEngine::meshCache() { return _meshCache; }
ConstantBufferManager* RenderEngine::constantBufferManager() { return _constantBufferManager; }
MaterialCache*         RenderEngine::materialCache() { return _materialCache; }
DebugDrawInterface*    RenderEngine::debugDraw() { return _renderers.debug.get(); }
AnimationCache*        RenderEngine::animationCache() { return _animationCache; }
