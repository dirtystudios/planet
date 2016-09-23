#pragma once

#include "Renderer.h"
#include "Rectangle.h"
#include "DebugRenderObj.h"
#include "DrawItem.h"
#include "Renderer.h"
#include "StateGroup.h"

#include <memory>
#include <vector>

class DebugRenderer : public TypedRenderer<DebugRenderObj> {
private:
    gfx::BufferId _rectVertexBuffer;
    size_t _rectBufferOffset;

    ConstantBuffer* _viewData{ nullptr };

    std::vector<DebugRenderObj*> _objs;
    const gfx::StateGroup* _base{ nullptr };

    void SetRect2DVertices(DebugRect2DRenderObj*);
    const gfx::DrawItem* DebugRenderer::CreateRect2DDrawItem(DebugRect2DRenderObj* renderObj);

    void RegisterRect2D(DebugRect2DRenderObj* rect);
public:

    void OnInit() final;

    void Register(DebugRenderObj* debugRO);
    void Unregister(DebugRenderObj* debugRO);

    void Submit(RenderQueue* renderQueue, RenderView* renderView) final;
};
