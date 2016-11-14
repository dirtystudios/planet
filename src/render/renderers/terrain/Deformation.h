#pragma once

#include <glm/glm.hpp>

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
