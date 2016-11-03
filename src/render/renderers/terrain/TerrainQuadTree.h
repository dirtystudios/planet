#pragma once

#include <glm/glm.hpp>
#include <memory>
#include "Rectangle.h"
#include "TerrainTileKey.h"

enum class TerrainDeformationType : uint8_t { None, Spherical };

class TerrainDeformation {
private:
    const TerrainDeformationType _type;

public:
    TerrainDeformation(TerrainDeformationType type) : _type(type) {}

    virtual glm::vec3 worldToDeformed(const glm::vec3& world) = 0;
};

class NoDeformation : public TerrainDeformation {
public:
    NoDeformation() : TerrainDeformation(TerrainDeformationType::None) {}

    virtual glm::vec3 worldToDeformed(const glm::vec3& world) final { return world; }
};

class SphericalDeformation : public TerrainDeformation {
private:
    float _radius{0.f};

public:
    SphericalDeformation(float radius) : TerrainDeformation(TerrainDeformationType::Spherical), _radius(radius) {}

    virtual glm::vec3 worldToDeformed(const glm::vec3& world) final { return glm::normalize(world) * _radius; }

    float radius() const { return _radius; };
};

class TerrainQuadNode;

class TerrainQuadTree {
private:
    static uint32_t tid() {
        static uint32_t tid = 0;
        return tid++;
    }
    const uint32_t                      _terrainId{tid()};
    std::unique_ptr<TerrainQuadNode>    _rootNode;
    glm::mat4                           _transform;
    glm::mat4                           _sampleSpaceTransform;
    std::shared_ptr<TerrainDeformation> _deformation;

public:
    TerrainQuadTree(double rootNodeSize, const std::shared_ptr<TerrainDeformation>& deformation, const glm::mat4& transform = glm::mat4());

    TerrainQuadNode* rootNode();

    glm::dvec3 localToDeformed(const glm::dvec3& localPosition);
    glm::dvec3 localToWorld(const glm::dvec3& localPosition);
    glm::mat4& transform();

    dm::Rect2Dd localRectForKey(const TerrainTileKey& key);
    dm::Rect3Dd worldRectForKey(const TerrainTileKey& key);
};

using TerrainQuadTreePtr = std::shared_ptr<TerrainQuadTree>;
