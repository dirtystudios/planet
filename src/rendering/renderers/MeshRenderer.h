#pragma once

#include "Renderer.h"

class MeshRenderer : public Renderer {
private:    
    graphics::PipelineStateId _defaultPS{0};
    graphics::ShaderParamId _transform{0};    
    graphics::ShaderParamId _normTransform{0};
public:
    ~MeshRenderer();

    void OnInit() override;
    RenderObj* Register(SimObj* simObj) final;
    void Unregister(RenderObj* renderObj) final;
    void Submit(RenderQueue* renderQueue, RenderView* renderView) final;
};