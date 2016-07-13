#pragma once

#include "Renderer.h"
#include "SkyboxRenderObj.h"
#include "StateGroup.h"
class SkyRenderer : public Renderer {
private:
    std::vector<SkyboxRenderObj*> _objs;
    const gfx::StateGroup* _base;

public:
    ~SkyRenderer();

    void OnInit() override;
    RenderObj* Register(SimObj* simObj) final;
    void Unregister(RenderObj* renderObj) final;
    void Submit(RenderQueue* renderQueue, RenderView* renderView) final;
};
