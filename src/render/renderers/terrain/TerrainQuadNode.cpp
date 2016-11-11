#include "TerrainQuadNode.h"
#include "TerrainQuadTree.h"
#
using namespace dm;

TerrainQuadNode::TerrainQuadNode(TerrainQuadTree* terrain, const TerrainTileKey& key, const dm::Rect3Dd& local, const dm::Rect3Dd& sampleSpace, double size)
    : terrain(terrain), key(key), size(size), localRect(local), sampleRect(sampleSpace) {}

glm::mat4 TerrainQuadNode::worldMatrix() const {
    dm::Transform transform;
    transform.translate(localRect.center());
    transform.scale(glm::vec3(size / 2.f, size / 2.f, 1.f)); // dont scale Z because thats the direction we displace for NoDeformation

    return terrain->transform() * transform.matrix();
}

glm::dvec3 TerrainQuadNode::normal() const {
    glm::dvec3 bl = worldRect().bl();
    glm::dvec3 tl = worldRect().tl();
    glm::dvec3 br = worldRect().br();

    return glm::normalize(glm::cross(tl - bl, br - bl));
}

glm::dvec3 TerrainQuadNode::deformedPosition() const { return terrain->localToDeformed(localRect.center()); }

dm::Rect3Dd TerrainQuadNode::worldRect() const { return terrain->worldRectForKey(key); }

void TerrainQuadNode::SubdivideIfNecessary() {
    if (children.size() > 0) {
        return;
    }

    std::array<Rect3Dd, 4>        childrenRects       = localRect.subdivide();
    std::array<Rect3Dd, 4>        childrenSampleRects = sampleRect.subdivide();
    std::array<TerrainTileKey, 4> childrenKeys        = key.subdivide();

    children.reserve(4);
    for (uint32_t i = 0; i < 4; ++i) {
        children.emplace_back(this->terrain, childrenKeys[i], childrenRects[i], childrenSampleRects[i], size / 2.0);
    }
}
