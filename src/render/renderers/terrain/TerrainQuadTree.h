#pragma once

#include <glm/glm.hpp>
#include <memory>
#include "Rectangle.h"
#include "TerrainTileKey.h"
#include "Deformation.h"

class TerrainQuadNode;

class TerrainQuadTree {
private:
    static uint32_t tid() {
        static uint32_t tid = 0;
        return tid++;
    }
    const uint32_t _terrainId{tid()};
    std::unique_ptr<TerrainQuadNode> _rootNode;
    glm::mat4 _transform;
    glm::mat4 _sampleSpaceTransform;
    std::shared_ptr<TerrainDeformation> _deformation;

public:
    TerrainQuadTree(double rootNodeSize, const std::shared_ptr<TerrainDeformation>& deformation, const glm::mat4& transform = glm::mat4());

    TerrainQuadNode* rootNode();

    glm::dvec3 localToDeformed(const glm::dvec3& localPosition) const;
    glm::dvec3 localToWorld(const glm::dvec3& localPosition) const;
    glm::mat4& transform();

    glm::mat4 worldTransformForKey(const TerrainTileKey& key) const;

    dm::Rect3Dd sampleRectForKey(const TerrainTileKey& key) const;
    dm::Rect3Dd localRectForKey(const TerrainTileKey& key) const;
    dm::Rect3Dd worldRectForKey(const TerrainTileKey& key) const;

    uint32_t terrainId() const { return _terrainId; };
};

using TerrainQuadTreePtr = std::shared_ptr<TerrainQuadTree>;
