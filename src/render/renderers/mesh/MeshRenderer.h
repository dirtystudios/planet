#pragma once

#include "Renderer.h"
#include "MeshRenderData.h"
#include <memory>
#include <vector>

class MeshRenderer : public Renderer {
private:
	std::vector<MeshRenderData> meshRenderData;
	std::unique_ptr<const gfx::StateGroup> baseStateGroup;
	std::vector<std::unique_ptr<const gfx::DrawItem>>  _drawItems;
public:
    ~MeshRenderer();

    void OnInit() override;
    void Register(RenderObj* skyRO) {}
    void Unregister(RenderObj* skyRO) {}
    void Submit(RenderQueue* renderQueue, const FrameView* view) final;
};
