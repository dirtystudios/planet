#pragma once

#include "Renderer.h"
#include "RendererType.h"
#include "RenderObj.h"
#include "RenderView.h"
#include "RenderDevice.h"
#include <unordered_map>
#include "ShaderCache.h"
#include "PipelineStateCache.h"
#include "RenderServiceLocator.h"
#include "VertexLayoutCache.h"
#include "MeshCache.h"
#include "SimObj.h"
#include "ConstantBufferManager.h"
#include "StateGroup.h"



class RenderEngine : public RenderServiceLocator {
private:
    gfx::RenderDevice* _device{nullptr};
    RenderView* _view{nullptr};
    std::unordered_map<RendererType, Renderer*> _renderers;
    std::unordered_map<RenderObj*, Renderer*> _renderObjLookup;
    ShaderCache* _shaderCache{nullptr};
    PipelineStateCache* _pipelineStateCache{nullptr};
    VertexLayoutCache* _vertexLayoutCache{nullptr};
    MeshCache* _meshCache;
    ConstantBufferManager* _constantBufferManager{nullptr};
    MaterialCache* _materialCache;
    
    
    const gfx::StateGroup* _stateGroupDefaults{nullptr};
public:
    RenderEngine(gfx::RenderDevice* device, RenderView* view);
    ~RenderEngine();

    RenderObj* Register(SimObj* simObj, RendererType rendererType);
    void Unregister(RenderObj* renderObj);
    void RenderFrame();

    ShaderCache* GetShaderCache() override;
    PipelineStateCache* GetPipelineStateCache() override;
    gfx::RenderDevice* GetRenderDevice() override;
    VertexLayoutCache* GetVertexLayoutCache() override;
    MeshCache* GetMeshCache() override;
    ConstantBufferManager* GetConstantBufferManager() override;
    MaterialCache* GetMaterialCache() override;

    // TODO:: Making this temporarily visible for UIManager until we figure out object model
    Renderer* GetRenderer(RendererType type);

private:
};
