#pragma once

#include "TerrainRenderObj.h"

class FlatTerrain : public TerrainRenderObj {
private:
    TerrainQuadTree* _quadTree;

public:
    FlatTerrain(double size) { _quadTree = addQuadTree(size, std::make_shared<NoDeformation>(), glm::mat4()); }
    ~FlatTerrain() {}

    void setTransform(const glm::mat4& transform) { _quadTree->transform() = transform; }
};
