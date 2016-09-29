#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include "Hash.h"
#include "Rectangle.h"
#include "TerrainTileKey.h"

class TerrainQuadTree;

using LocalCoords = glm::dvec2;

class TerrainQuadNode {
public:
    TerrainQuadNode(TerrainQuadTree* terrain, const TerrainTileKey& key, const glm::dvec3& local, float size) : terrain(terrain), key(key), local(local), size(size) {}

    TerrainQuadTree*             terrain{nullptr};
    TerrainTileKey               key;
    LocalCoords                  local;
    float                        size;
    std::vector<TerrainQuadNode> children;

    bool HasChildren() const { return children.size() == 4; }
    void SubdivideIfNecessary();

    glm::dvec3 worldCoordinates() const;

    // these dont need to be functions
    dm::Rect3Df worldRect() const;
    dm::Rect2Df localRect() const;
};
