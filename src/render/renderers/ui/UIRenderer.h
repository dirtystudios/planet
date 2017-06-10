#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Config.h"
#include "Helpers.h"
#include "RenderDevice.h"
#include "Renderer.h"
#include "StateGroupEncoder.h"
#include "UIRenderObj.h"

class UIRenderer : public TypedRenderer<UIFrameRenderObj> {
private:
    gfx::BufferId                  _vertexBuffer;
    size_t                         _bufferOffset;
    size_t                         _bufferSize;
    ConstantBuffer*                _viewData{nullptr};
    ConstantBuffer*                _viewData3D{ nullptr };
    std::vector<UIFrameRenderObj*> _objs; // TODO: Renderers shouldnt store their objs
    const gfx::StateGroup*         _base{nullptr};
    const gfx::StateGroup*         _base3D{ nullptr };

public:
    UIRenderer() : TypedRenderer<UIFrameRenderObj>(RendererType::Ui) {};
    ~UIRenderer();

    void Register(UIFrameRenderObj* uiFrameObj);
    void Unregister(UIFrameRenderObj* uiFrameObj);

    // Renderer Interface
    std::vector<RenderPassType> supportedPasses() const final { return { RenderPassType::Standard }; }
    void OnInit() final;
    void Submit(RenderQueue* renderQueue, const FrameView* view) final;
};
