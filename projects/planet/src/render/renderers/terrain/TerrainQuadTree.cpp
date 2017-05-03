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

glm::dvec3 TerrainQuadTree::localToDeformed(const glm::dvec3& localPosition) const { return _deformation->worldToDeformed(localToWorld(localPosition)); }

glm::dvec3 TerrainQuadTree::localToWorld(const glm::dvec3& localPosition) const {
    glm::dvec4 res = _transform * glm::dvec4(localPosition.x, localPosition.y, 0, 1);
    return glm::dvec3(res);
}
glm::mat4& TerrainQuadTree::transform() { return _transform; }

glm::mat4 TerrainQuadTree::worldTransformForKey(const TerrainTileKey& key) const {
    dg_assert_nm(key.tid == _terrainId);

    dm::Rect3Dd localRect = localRectForKey(key);
    double      scale     = _rootNode->size / dm::pow<double>(2.0, key.lod + 1);

    dm::Transform keyTransform;
    keyTransform.translate(localRect.center());
    keyTransform.scale(glm::vec3(scale, scale, 1.0)); // dont scale Z because thats the direction we displace for deformation

    return _transform * keyTransform.matrix();
}

dm::Rect3Dd TerrainQuadTree::sampleRectForKey(const TerrainTileKey& key) const {
    dg_assert_nm(key.tid == _terrainId);
    dm::Rect3Dd localRect = localRectForKey(key);
    return dm::Rect3Dd(localRect, _sampleSpaceTransform);
}

dm::Rect3Dd TerrainQuadTree::localRectForKey(const TerrainTileKey& key) const {
    dg_assert_nm(key.tid == _terrainId);
    return ::localRectForKey(key, _rootNode->size);
}

dm::Rect3Dd TerrainQuadTree::worldRectForKey(const TerrainTileKey& key) const {
    dg_assert_nm(key.tid == _terrainId);
    dm::Rect3Dd localRect = localRectForKey(key);
    return dm::Rect3Dd(localRect, _transform);
}
