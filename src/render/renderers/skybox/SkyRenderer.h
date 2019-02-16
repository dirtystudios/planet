#pragma once

#include "Renderer.h"
#include "SkyboxRenderObj.h"
#include "StateGroup.h"

class SkyRenderer : public TypedRenderer<SkyboxRenderObj> {
private:
    std::vector<SkyboxRenderObj*> _objs;
    const gfx::StateGroup*        _base;

public:
    SkyRenderer() : TypedRenderer<SkyboxRenderObj>(RendererType::Skybox) {}
    virtual ~SkyRenderer() final;

    void Register(SkyboxRenderObj* skyRO) final;
    void Unregister(SkyboxRenderObj* skyRO) final;

    void OnInit() override;
    void Submit(RenderQueue* renderQueue, const FrameView* view) final;
    void Submit(ComputeQueue* renderQueue, const FrameView* view) final {};
};
