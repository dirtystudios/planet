#pragma once

#include <unordered_map>
#include "AnimationCache.h"
#include "ConstantBufferManager.h"
#include "DebugDrawInterface.h"
#include "MeshCache.h"
#include "PipelineStateCache.h"
#include "RayTraceRenderer.h"
#include "RenderDevice.h"
#include "RenderObj.h"
#include "RenderServiceLocator.h"
#include "RenderView.h"
#include "Renderer.h"
#include "RendererType.h"
#include "ShaderCache.h"
#include "SimObj.h"
#include "StateGroup.h"
#include "TerrainRenderer.h"
#include "VertexLayoutCache.h"
#include "Swapchain.h"

 struct RenderScene {
    std::vector<RenderObj*> renderObjects;
};

class SkyRenderer;
class TextRenderer;
class MeshRenderer;
class UIRenderer;
class DebugRenderer;
class TerrainRenderer;
class RayTraceRenderer;

struct Renderers {
    std::unique_ptr<SkyRenderer>     sky;
    std::unique_ptr<TextRenderer>    text;
    std::unique_ptr<MeshRenderer>    mesh;
    std::unique_ptr<UIRenderer>      ui;
    std::unique_ptr<DebugRenderer>   debug;
    std::unique_ptr<TerrainRenderer> terrain;
    std::unique_ptr<RayTraceRenderer> raytrace;
};


class RenderEngine : public RenderServiceLocator {
private:
    gfx::RenderDevice* _device{nullptr};
    gfx::Swapchain* _swapchain{nullptr};
    RenderView*        _view{nullptr};
    std::unordered_map<RendererType, Renderer*> _renderersByType;
    std::unordered_map<RenderObj*, Renderer*>   _renderObjLookup;
    ShaderCache*           _shaderCache{nullptr};
    PipelineStateCache*    _pipelineStateCache{nullptr};
    VertexLayoutCache*     _vertexLayoutCache{nullptr};
    MeshCache*             _meshCache;
    ConstantBufferManager* _constantBufferManager{nullptr};
    MaterialCache*         _materialCache;
    AnimationCache*        _animationCache;

    Renderers _renderers;

    const gfx::StateGroup* _stateGroupDefaults{nullptr};

public:
    RenderEngine(gfx::RenderDevice* device, gfx::Swapchain* swapchain, RenderView* view);
    ~RenderEngine();

    Renderers& Renderers() { return _renderers; }
    void       RenderFrame(const RenderScene* scene);
    void       CreateRenderTargets();
    RenderPassId _baseRenderPass { gfx::NULL_ID };
    gfx::TextureId _depthBuffer { gfx::NULL_ID };
    
    ShaderCache*           shaderCache() override;
    PipelineStateCache*    pipelineStateCache() override;
    VertexLayoutCache*     vertexLayoutCache() override;
    MeshCache*             meshCache() override;
    ConstantBufferManager* constantBufferManager() override;
    MaterialCache*         materialCache() override;
    DebugDrawInterface*    debugDraw() override;
    AnimationCache*        animationCache() override;

private:
};
