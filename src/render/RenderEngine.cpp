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

    _renderers.mesh->SetActive(true);
    _renderers.sky->SetActive(false);
    _renderers.terrain->SetActive(false);
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

    viewConstantsBuffer = _constantBufferManager->GetConstantBuffer(sizeof(ViewConstants), "ViewConstants");
    cmdbuf              = _device->CreateCommandBuffer();
    for (auto p : _renderersByType) {
        LOG_D("Initializing Renderer: %d", p.first);
        p.second->Init(_device, this);
    }

    StateGroupEncoder encoder;
    encoder.Begin();
    encoder.SetBlendState(BlendState());
    encoder.SetRasterState(RasterState());
    encoder.SetDepthState(DepthState());
    encoder.BindResource(viewConstantsBuffer->GetBinding(0));
    _stateGroupDefaults = encoder.End();
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

    assert(_view);
    FrameView view = _view->frameView();
    view._visibleObjects = scene->renderObjects; // for now
    
    // update view constants
    ViewConstants* mapped = viewConstantsBuffer->Map<ViewConstants>();
    mapped->eye           = view.eyePos;
    mapped->proj          = view.projection;
    mapped->view          = view.view;
    viewConstantsBuffer->Unmap();
    for (const std::pair<RendererType, Renderer*>& p : _renderersByType) {
        if (p.second->isActive()) {
            p.second->Submit(&queue, &view);
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
