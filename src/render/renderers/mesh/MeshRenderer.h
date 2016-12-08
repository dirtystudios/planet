#pragma once

#include "Renderer.h"
#include "MeshRenderObj.h"
#include <memory>
#include <vector>
#include <unordered_map>

class MeshRenderer : public Renderer {
private:
    std::vector<MeshRenderObj*> meshRenderObjs;
    std::vector<std::unique_ptr<const gfx::DrawItem>>  _drawItems;

    std::unordered_multimap<uint32_t, const gfx::DrawItem*> sortedMatCache;

    gfx::TextureId vertBufferId;
    gfx::TextureId indexBufferId;

    uint32_t vertOffset{0};
    uint32_t indexOffset{0};

public:
    ~MeshRenderer();

    void OnInit() override;
    void Register(MeshRenderObj* renderObj);
    void Unregister(MeshRenderObj* renderObj) { assert(false); }
    void Submit(RenderQueue* renderQueue, const FrameView* view) final;
};
