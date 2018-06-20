#pragma once

#include <unordered_map>
#include "AnimationCache.h"
#include "ConstantBufferManager.h"
#include "DebugDrawInterface.h"
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
#include "TerrainRenderer.h"
#include "VertexLayoutCache.h"

 struct RenderScene {
    std::vector<RenderObj*> renderObjects;
};
//

// enum class RenderLayerType : uint8_t {
//    World,
//    HUD
//};
//
// constexpr uint32_t kRenderLayerCount = 2;
//
//// RenderViews support RenderStages
//// Renderers support RenderViews
//// RenderObjects are registered with RenderStages
//
//
// class RenderSceneManager {
// public:
//    std::array<std::vector<RenderObj*>, kRenderLayerCount> _objsByLayer;
// public:
//    void Register(RenderObj* renderObj, RenderLayerType layer);
//    void Unregister(RenderObj* renderObj);
//
//    void Render() {
//
//    }
//};
//
// struct RenderViews {
//    std::unique_ptr<PlayerRenderView> player;
//    std::unique_ptr<HUDRenderView> hud;
//};


class SkyRenderer;
class TextRenderer;
class MeshRenderer;
class UIRenderer;
class DebugRenderer;
class TerrainRenderer;

struct Renderers {
    std::unique_ptr<SkyRenderer>     sky;
    std::unique_ptr<TextRenderer>    text;
    std::unique_ptr<MeshRenderer>    mesh;
    std::unique_ptr<UIRenderer>      ui;
    std::unique_ptr<DebugRenderer>   debug;
    std::unique_ptr<TerrainRenderer> terrain;
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
    RenderPassId _baseRenderPass;
    
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
