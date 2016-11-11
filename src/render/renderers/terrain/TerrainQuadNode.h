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
    TerrainQuadNode(TerrainQuadTree* terrain, const TerrainTileKey& key, const dm::Rect3Dd& local, const dm::Rect3Dd& sampleSpace, double size);

    TerrainQuadTree*             terrain{nullptr};
    std::vector<TerrainQuadNode> children;
    const TerrainTileKey         key;
    const double                 size;
    const dm::Rect3Dd            localRect;
    const dm::Rect3Dd            sampleRect;

    glm::dvec3  deformedPosition() const;
    glm::dvec3  normal() const;
    dm::Rect3Dd worldRect() const;
    glm::mat4   worldMatrix() const;
    bool        HasChildren() const { return children.size() == 4; }
    void        SubdivideIfNecessary();

private:
};
