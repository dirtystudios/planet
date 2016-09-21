#pragma once

#include "Renderer.h"

class MeshRenderer : public Renderer {
private:
public:
    ~MeshRenderer();

    void OnInit() override;
    void Register(RenderObj* skyRO) {}
    void Unregister(RenderObj* skyRO) {}
    void Submit(RenderQueue* renderQueue, RenderView* renderView) final;
};
