#pragma once

#include <vector>
#include "RenderObj.h"
#include "TerrainQuadTree.h"

class TerrainRenderer;

class TerrainRenderObj : public RenderObj {
private:
    friend TerrainRenderer;

private:
    std::vector<std::unique_ptr<TerrainQuadTree>> _trees;
    std::vector<TerrainQuadTree*> _weakPtrs;

public:
    TerrainRenderObj()
        : RenderObj(RendererType::Terrain) {}
    virtual ~TerrainRenderObj() {}

protected:
    template <typename... Args>
    TerrainQuadTree* addQuadTree(Args&&... args) {
        _trees.push_back(std::make_unique<TerrainQuadTree>(std::forward<Args>(args)...));
        _weakPtrs.emplace_back(_trees.back().get());
        return _weakPtrs.back();
    }

    const std::vector<TerrainQuadTree*>& getQuadTrees() const { return _weakPtrs; }
};
