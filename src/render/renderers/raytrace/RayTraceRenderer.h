#pragma once

#include "Renderer.h"
#include "MeshRenderObj.h"
#include <memory>
#include <vector>
#include <unordered_map>

class RayTraceRenderer : public Renderer {
private:
    std::vector<MeshRenderObj*> meshRenderObjs;
    BufferId csVertBuffer{ NULL_ID };
    BufferId fakeVertBuff{ NULL_ID };
    ConstantBuffer* cbPerObj{ nullptr };
    ConstantBuffer* rpCbPerObj{ nullptr };
    TextureId computeResultTex{ NULL_ID };
    TextureId skyboxTextureId{ NULL_ID };
    BufferId sphereBuff{ NULL_ID };
    const gfx::StateGroup* csStateGroup{ nullptr };
    const gfx::StateGroup* renderStateGroup{ nullptr };

    std::unique_ptr<FrameView> lastFrameView = std::make_unique<FrameView>();
    unsigned int currentSample = 0;

    std::vector<std::unique_ptr<const gfx::DispatchItem>> _dispatchItems;
    std::vector<std::unique_ptr<const gfx::DrawItem>> _drawItems;

    void SetupSpheres();
public:
    RayTraceRenderer() : Renderer(RendererType::RayTrace) {}
    ~RayTraceRenderer();

    void OnInit() override;
    void Register(MeshRenderObj* renderObj);
    void Unregister(MeshRenderObj* renderObj) { assert(false); }
    void Submit(RenderQueue* renderQueue, const FrameView* view) final;
    void Submit(ComputeQueue* renderQueue, const FrameView* view) final;
};
