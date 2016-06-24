#pragma once

#include "Renderer.h"
#include "ChunkedTerrainRenderObj.h"

class ChunkedTerrainRenderer : public Renderer {
private:
    std::vector<ChunkedTerrainRenderObj*> _objs;
    gfx::PipelineStateId _defaultPS{0};
    gfx::ShaderParamId _transform{0};

    const uint32_t kQuadResolution{2};
    const float kSplitFactor{1.5f};
    const uint32_t kMaxLOD{7};
    const uint32_t kQuadTreeNodes{4};
    const uint32_t kGPUTextureBufferSize{512};

public:
    ~ChunkedTerrainRenderer();

    void OnInit() override;
    RenderObj* Register(SimObj* simObj) final;
    void Unregister(RenderObj* renderObj) final;
    void Submit(RenderQueue* renderQueue, RenderView* renderView) final;
};
