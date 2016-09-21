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
    std::vector<UIFrameRenderObj*> _objs; // TODO: Renderers shouldnt store their objs
    const gfx::StateGroup*         _base{nullptr};

public:
    ~UIRenderer();

    void Register(UIFrameRenderObj* uiFrameObj);
    void Unregister(UIFrameRenderObj* uiFrameObj);

    // Renderer Interface
    void OnInit() final;
    void Submit(RenderQueue* renderQueue, RenderView* renderView) final;
};
