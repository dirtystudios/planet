#include "RenderEngine.h"
#include <cassert>
#include "Config.h"
#include "ChunkedTerrainRenderer.h"
#include "SkyRenderer.h"
#include "MeshRenderer.h"
#include "TextRenderer.h"
#include "ConstantBuffer.h"
#include "UIRenderer.h"

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
    // _renderers.insert({RendererType::ChunkedTerrain, new ChunkedTerrainRenderer()});
    _renderers.insert({RendererType::Skybox, new SkyRenderer()});
    //    _renderers.insert({RendererType::Mesh, new MeshRenderer()});
    _renderers.insert({RendererType::Ui, new UIRenderer()});
    _renderers.insert({RendererType::Text, new TextRenderer()});

    std::string shaderDirPath =
        config::Config::getInstance().GetConfigString("RenderDeviceSettings", "ShaderDirectory");
    std::string assetDirPath = config::Config::getInstance().GetConfigString("RenderDeviceSettings", "AssetDirectory");

    _shaderCache           = new ShaderCache(_device, shaderDirPath);
    _pipelineStateCache    = new PipelineStateCache(_device);
    _vertexLayoutCache     = new VertexLayoutCache(_device);
    _meshCache             = new MeshCache(_device, assetDirPath);
    _constantBufferManager = new ConstantBufferManager(_device);
    _materialCache      = new MaterialCache(_device, assetDirPath);

   
    viewConstantsBuffer = _constantBufferManager->GetConstantBuffer(sizeof(ViewConstants));
    cmdbuf = _device->CreateCommandBuffer();
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
    cmdbuf->Reset();          
    RenderQueue queue(cmdbuf);    
    
    
    cmdbuf->Clear(1, 0, 0, 1);
    
    // update view constants
    ViewConstants* mapped = viewConstantsBuffer->Map<ViewConstants>();
    mapped->eye = glm::vec3(1, 0, 0);
    mapped->proj = _view->camera->BuildProjection();
    mapped->view = _view->camera->BuildView();
    viewConstantsBuffer->Unmap();
    
    
    
    cmdbuf->BindResource(viewConstantsBuffer->GetBinding(0));
    

    assert(_view);
    for (const std::pair<RendererType, Renderer*>& p : _renderers) {
        p.second->Submit(&queue, _view);
    }
    queue.Submit(_device);
    _device->RenderFrame();    
}

ShaderCache* RenderEngine::GetShaderCache() { return _shaderCache; }

PipelineStateCache* RenderEngine::GetPipelineStateCache() { return _pipelineStateCache; }

gfx::RenderDevice* RenderEngine::GetRenderDevice() { return _device; }

VertexLayoutCache* RenderEngine::GetVertexLayoutCache() { return _vertexLayoutCache; }
MeshCache* RenderEngine::GetMeshCache() { return _meshCache; }
ConstantBufferManager* RenderEngine::GetConstantBufferManager() { return _constantBufferManager; }
MaterialCache* RenderEngine::GetMaterialCache() { return _materialCache; }

Renderer* RenderEngine::GetRenderer(RendererType type) {
    auto it = _renderers.find(type);
    return (it == _renderers.end() ? nullptr : it->second);
}
