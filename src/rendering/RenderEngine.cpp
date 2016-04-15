#include "RenderEngine.h"
#include <cassert>
#include "Config.h"
#include "ChunkedTerrainRenderer.h"
#include "SkyRenderer.h"
#include "MeshRenderer.h"

using namespace graphics;

RenderEngine::RenderEngine(RenderDevice* device, RenderView* view) : _device(device), _view(view) {
    // _renderers.insert({RendererType::ChunkedTerrain, new ChunkedTerrainRenderer()});
    // _renderers.insert({RendererType::Skybox, new SkyRenderer()});    
    _renderers.insert({RendererType::Mesh, new MeshRenderer()});

    std::string shaderDirPath =
        config::Config::getInstance().GetConfigString("RenderDeviceSettings", "ShaderDirectory");
    std::string assetDirPath = "";

    _shaderCache        = new ShaderCache(_device, shaderDirPath);
    _pipelineStateCache = new PipelineStateCache(_device);
    _vertexLayoutCache  = new VertexLayoutCache(_device);
    _meshCache          = new MeshCache(_device, assetDirPath);

    for (auto p : _renderers) {
        LOG_D("Initializing Renderer: %d", p.first);
        p.second->Init(this);
    }
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

    for (auto it : _renderers) {
        delete it.second;
    }

    _renderers.clear();
}

RenderObj* RenderEngine::Register(SimObj* simObj, RendererType rendererType) {
    Renderer* renderer = GetRenderer(rendererType);
    assert(renderer);

    RenderObj* renderObj = renderer->Register(simObj);
    assert(renderObj);
    return renderObj;
}

void RenderEngine::Unregister(RenderObj* renderObj) {
    Renderer* renderer = GetRenderer(renderObj->GetRendererType());
    assert(renderer);

    renderer->Unregister(renderObj);
}

void RenderEngine::RenderFrame() {
    Frame* frame = _device->BeginFrame();
    RenderQueue queue(frame);

    assert(_view);
    for (const std::pair<RendererType, Renderer*>& p : _renderers) {
        p.second->Submit(&queue, _view);
    }

    queue.Submit();

    _device->SubmitFrame();
}

ShaderCache* RenderEngine::GetShaderCache() { return _shaderCache; }

PipelineStateCache* RenderEngine::GetPipelineStateCache() { return _pipelineStateCache; }

graphics::RenderDevice* RenderEngine::GetRenderDevice() { return _device; }

VertexLayoutCache* RenderEngine::GetVertexLayoutCache() { return _vertexLayoutCache; }
MeshCache* RenderEngine::GetMeshCache() { return _meshCache; }

Renderer* RenderEngine::GetRenderer(RendererType type) {
    auto it = _renderers.find(type);
    return (it == _renderers.end() ? nullptr : it->second);
}