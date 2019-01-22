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
    BufferId vertBuffer{ NULL_ID };
    TextureId resultTex{ NULL_ID };
    const gfx::StateGroup* stateGroup{ nullptr };
public:
    RayTraceRenderer() : Renderer(RendererType::RayTrace) {}
    ~RayTraceRenderer();

    void OnInit() override;
    void Register(MeshRenderObj* renderObj);
    void Unregister(MeshRenderObj* renderObj) { assert(false); }
    void Submit(RenderQueue* renderQueue, const FrameView* view) final;
    void Submit(ComputeQueue*) final;
};
