#pragma once

#include "Renderer.h"
#include "SkyboxRenderObj.h"

class SkyRenderer : public Renderer {
private:
    gfx::PipelineStateId _defaultPS;
    std::vector<SkyboxRenderObj*> _objs;

public:
    ~SkyRenderer();

    void OnInit() override;
    RenderObj* Register(SimObj* simObj) final;
    void Unregister(RenderObj* renderObj) final;
    void Submit(RenderQueue* renderQueue, RenderView* renderView) final;
};
