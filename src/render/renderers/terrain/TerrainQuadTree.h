#pragma once

#include <glm/glm.hpp>
#include <memory>

class TerrainQuadNode;

class TerrainQuadTree {
private:
    static uint32_t tid() {
        static uint32_t tid = 0;
        return tid++;
    }
    const uint32_t                   _terrainId{tid()};
    std::unique_ptr<TerrainQuadNode> _rootNode;
    glm::mat4                        _transform;

public:
    TerrainQuadTree(float rootNodeSize, const glm::mat4& transform = glm::mat4());

    TerrainQuadNode* rootNode();

    glm::vec3 localToWorld(const glm::vec2& localPosition);
    glm::mat4 transform() const;
};

using TerrainQuadTreePtr = std::shared_ptr<TerrainQuadTree>;
