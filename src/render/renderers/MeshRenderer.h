#pragma once

#include "Renderer.h"

class MeshRenderer : public Renderer {
private:    
    gfx::PipelineStateId _defaultPS{0};
    gfx::ShaderParamId _transform{0};    
    gfx::ShaderParamId _normTransform{0};
public:
    ~MeshRenderer();

    void OnInit() override;
    RenderObj* Register(SimObj* simObj) final;
    void Unregister(RenderObj* renderObj) final;
    void Submit(RenderQueue* renderQueue, RenderView* renderView) final;
};
