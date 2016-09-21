#pragma once

#include <unordered_map>
#include "ConstantBufferManager.h"
#include "MeshCache.h"
#include "PipelineStateCache.h"
#include "RenderDevice.h"
#include "RenderObj.h"
#include "RenderServiceLocator.h"
#include "RenderView.h"
#include "Renderer.h"
#include "RendererType.h"
#include "ShaderCache.h"
#include "SimObj.h"
#include "StateGroup.h"
#include "VertexLayoutCache.h"

//
// struct RenderScene {
//    using RenderObjIdx = uint32_t;
//    std::vector<RenderView*> _activeViews;
//    std::vector<std::vector<RenderObjIdx>> _objectsPerView;
//
//    std::vector<RenderObj*> _visibleObjects;
//};
//

class SkyRenderer;
class TextRenderer;
class MeshRenderer;
class UIRenderer;

struct Renderers {
    std::unique_ptr<SkyRenderer>  sky;
    std::unique_ptr<TextRenderer> text;
    std::unique_ptr<MeshRenderer> mesh;
    std::unique_ptr<UIRenderer>   ui;
};

class RenderEngine : public RenderServiceLocator {
private:
    gfx::RenderDevice* _device{nullptr};
    RenderView*        _view{nullptr};
    std::unordered_map<RendererType, Renderer*> _renderersByType;
    std::unordered_map<RenderObj*, Renderer*>   _renderObjLookup;
    ShaderCache*           _shaderCache{nullptr};
    PipelineStateCache*    _pipelineStateCache{nullptr};
    VertexLayoutCache*     _vertexLayoutCache{nullptr};
    MeshCache*             _meshCache;
    ConstantBufferManager* _constantBufferManager{nullptr};
    MaterialCache*         _materialCache;

    Renderers _renderers;

    const gfx::StateGroup* _stateGroupDefaults{nullptr};

public:
    RenderEngine(gfx::RenderDevice* device, RenderView* view);
    ~RenderEngine();

    Renderers& Renderers() { return _renderers; }
    void       RenderFrame();

    ShaderCache*           GetShaderCache() override;
    PipelineStateCache*    GetPipelineStateCache() override;
    gfx::RenderDevice*     GetRenderDevice() override;
    VertexLayoutCache*     GetVertexLayoutCache() override;
    MeshCache*             GetMeshCache() override;
    ConstantBufferManager* GetConstantBufferManager() override;
    MaterialCache*         GetMaterialCache() override;

private:
};
