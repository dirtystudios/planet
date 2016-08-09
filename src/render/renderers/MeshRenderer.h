#pragma once

#include "Renderer.h"

class MeshRenderer : public Renderer {
private:    
public:
    ~MeshRenderer();

    void OnInit() override;
    RenderObj* Register(SimObj* simObj) final;
    void Unregister(RenderObj* renderObj) final;
    void Submit(RenderQueue* renderQueue, RenderView* renderView) final;
};
