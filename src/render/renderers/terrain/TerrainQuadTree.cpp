#include "TerrainQuadTree.h"
#include <glm/gtx/transform.hpp>
#include "DGAssert.h"
#include "TerrainQuadNode.h"

TerrainQuadTree::TerrainQuadTree(double rootNodeSize, const std::shared_ptr<TerrainDeformation>& deformation, const glm::mat4& transform)
    : _transform(transform), _sampleSpaceTransform(transform) {
    _deformation                   = deformation;
    dm::Rect3Dd    sampleSpaceRect = {{rootNodeSize, rootNodeSize}, _sampleSpaceTransform};
    dm::Rect3Dd    localRect       = {{rootNodeSize, rootNodeSize}};
    TerrainTileKey key(_terrainId, 0, 0, 0);
    _rootNode.reset(new TerrainQuadNode(this, key, localRect, sampleSpaceRect, rootNodeSize));
    _rootNode->SubdivideIfNecessary();
}

TerrainQuadNode* TerrainQuadTree::rootNode() { return _rootNode.get(); }

glm::dvec3 TerrainQuadTree::localToDeformed(const glm::dvec3& localPosition) { return _deformation->worldToDeformed(localToWorld(localPosition)); }

glm::dvec3 TerrainQuadTree::localToWorld(const glm::dvec3& localPosition) {
    glm::dvec4 res = _transform * glm::dvec4(localPosition.x, localPosition.y, 0, 1);
    return glm::dvec3(res);
}
glm::mat4& TerrainQuadTree::transform() { return _transform; }

dm::Rect2Dd TerrainQuadTree::localRectForKey(const TerrainTileKey& key) {
    dg_assert_nm(key.tid == _terrainId);
    return ::localRectForKey(key, _rootNode->size);
}

dm::Rect3Dd TerrainQuadTree::worldRectForKey(const TerrainTileKey& key) {
    dg_assert_nm(key.tid == _terrainId);
    return dm::Rect3Dd(localRectForKey(key), _transform);
}
