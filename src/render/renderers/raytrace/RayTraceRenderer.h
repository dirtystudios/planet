#pragma once

#include "Renderer.h"
#include "MeshRenderObj.h"
#include <memory>
#include <vector>
#include <unordered_map>

class RayTraceRenderer : public Renderer {
private:
    std::vector<MeshRenderObj*> meshRenderObjs;
    std::unique_ptr<MeshGeometry> sphereGeom{ nullptr };
    BufferId csVertBuffer{ NULL_ID };
    BufferId fakeVertBuff{ NULL_ID };
    TextureId resultTex{ NULL_ID };
    const gfx::StateGroup* csStateGroup{ nullptr };
    const gfx::StateGroup* renderStateGroup{ nullptr };

    std::vector<std::unique_ptr<const gfx::DispatchItem>> _dispatchItems;
    std::vector<std::unique_ptr<const gfx::DrawItem>> _drawItems;
public:
    RayTraceRenderer() : Renderer(RendererType::RayTrace) {}
    ~RayTraceRenderer();

    void OnInit() override;
    void Register(MeshRenderObj* renderObj);
    void Unregister(MeshRenderObj* renderObj) { assert(false); }
    void Submit(RenderQueue* renderQueue, const FrameView* view) final;
    void Submit(ComputeQueue*) final;
};
