#include "RenderEngine.h"
#include <cassert>
#include "Config.h"
#include "ChunkedTerrainRenderer.h"
#include "SkyRenderer.h"
#include "MeshRenderer.h"
#include "TextRenderer.h"
#include "ConstantBuffer.h"
#include "UIRenderer.h"
#include "StateGroupEncoder.h"

using namespace gfx;

struct ViewConstants {
    glm::vec3 eye;
    float padding0;
    glm::mat4 view;
    glm::mat4 proj;
};

// Todo::Where should view constantbuffer be...renderview?
using ViewConstantsBuffer = TypedConstantBuffer<ViewConstants>;
ConstantBuffer* viewConstantsBuffer;

CommandBuffer* cmdbuf;

RenderEngine::RenderEngine(RenderDevice* device, RenderView* view) : _device(device), _view(view) {
    // _renderers.insert({RendererType::ChunkedTerrain, new
    // ChunkedTerrainRenderer()});

    _renderers.sky.reset(new SkyRenderer());
    _renderers.text.reset(new TextRenderer());
    _renderers.ui.reset(new UIRenderer());
    _renderers.mesh.reset(new MeshRenderer());
    _renderersByType.insert({RendererType::Skybox, _renderers.sky.get()});
    _renderersByType.insert({RendererType::Mesh, _renderers.mesh.get()});
    _renderersByType.insert({RendererType::Ui, _renderers.ui.get()});
    _renderersByType.insert({RendererType::Text, _renderers.text.get()});

    _renderers.mesh->SetActive(false);
    _renderers.sky->SetActive(false);

    std::string shaderDirPath = config::Config::getInstance().GetConfigString("RenderDeviceSettings", "ShaderDirectory");
    std::string assetDirPath  = config::Config::getInstance().GetConfigString("RenderDeviceSettings", "AssetDirectory");

    _shaderCache           = new ShaderCache(_device, shaderDirPath);
    _pipelineStateCache    = new PipelineStateCache(_device);
    _vertexLayoutCache     = new VertexLayoutCache(_device);
    _meshCache             = new MeshCache(_device, assetDirPath);
    _constantBufferManager = new ConstantBufferManager(_device);
    _materialCache         = new MaterialCache(_device, assetDirPath);

    viewConstantsBuffer = _constantBufferManager->GetConstantBuffer(sizeof(ViewConstants));
    cmdbuf              = _device->CreateCommandBuffer();
    for (auto p : _renderersByType) {
        LOG_D("Initializing Renderer: %d", p.first);
        p.second->Init(this);
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

void RenderEngine::RenderFrame() {
    cmdbuf->Reset();
    RenderQueue queue(cmdbuf);
    queue.defaults = _stateGroupDefaults;

    // update view constants
    ViewConstants* mapped = viewConstantsBuffer->Map<ViewConstants>();
    mapped->eye           = _view->camera->pos;
    mapped->proj          = _view->camera->BuildProjection();
    mapped->view          = _view->camera->BuildView();
    viewConstantsBuffer->Unmap();

    assert(_view);
    for (const std::pair<RendererType, Renderer*>& p : _renderersByType) {
        if (p.second->IsActive()) {
            p.second->Submit(&queue, _view);
        }
    }
    queue.Submit(_device);
    _device->RenderFrame();
}

ShaderCache* RenderEngine::GetShaderCache() { return _shaderCache; }

PipelineStateCache* RenderEngine::GetPipelineStateCache() { return _pipelineStateCache; }

gfx::RenderDevice* RenderEngine::GetRenderDevice() { return _device; }

VertexLayoutCache*     RenderEngine::GetVertexLayoutCache() { return _vertexLayoutCache; }
MeshCache*             RenderEngine::GetMeshCache() { return _meshCache; }
ConstantBufferManager* RenderEngine::GetConstantBufferManager() { return _constantBufferManager; }
MaterialCache*         RenderEngine::GetMaterialCache() { return _materialCache; }
