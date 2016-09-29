#include "TerrainQuadTree.h"
#include <glm/gtx/transform.hpp>
#include "DGAssert.h"
#include "TerrainQuadNode.h"

TerrainQuadTree::TerrainQuadTree(float rootNodeSize, const glm::mat4& transform) : _transform(transform) {
    _rootNode.reset(new TerrainQuadNode(this, {_terrainId, 0, 0, 0}, {0, 0, 0}, rootNodeSize));
    _rootNode->SubdivideIfNecessary();
}

TerrainQuadNode* TerrainQuadTree::rootNode() { return _rootNode.get(); }

glm::vec3 TerrainQuadTree::localToWorld(const glm::vec2& localPosition) {
    glm::vec4 res = _transform * glm::vec4(localPosition.x, localPosition.y, 0, 1);
    return glm::vec3(res);
}
glm::mat4 TerrainQuadTree::transform() const { return _transform; }
