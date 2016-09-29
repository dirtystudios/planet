#include "TerrainQuadNode.h"
#include "TerrainQuadTree.h"

using namespace dm;

void TerrainQuadNode::SubdivideIfNecessary() {
    if (children.size() > 0) {
        return;
    }

    double childSize      = size / 2.0;
    double halfChildSize  = childSize / 2.0;
    double childPos[4][2] = {
        {local.x - halfChildSize, local.y + halfChildSize},
        {local.x + halfChildSize, local.y + halfChildSize},
        {local.x - halfChildSize, local.y - halfChildSize},
        {local.x + halfChildSize, local.y - halfChildSize},
    };

    children.reserve(4);
    for (uint32_t i = 0; i < 4; ++i) {
        glm::dvec3     childLocal;
        TerrainTileKey childKey;

        childLocal.x = childPos[i][0];
        childLocal.y = childPos[i][1];
        childLocal.z = 0;
        childKey.tid = key.tid;
        childKey.lod = key.lod + 1;
        childKey.tx  = key.tx * 2 + (i % 2 == 0 ? 0 : 1);
        childKey.ty  = key.ty * 2 + (i < 2 ? 1 : 0);
        //                    child->bbox.max = glm::vec3(child->pcoords.x + half_child_size, child->y + half_child_size, 0);
        //                    child->bbox.min = glm::vec3(child->pcoords.x - half_child_size, child->y - half_child_size, 0);

        children.emplace_back(this->terrain, childKey, childLocal, childSize);
    }
}

Rect3Df TerrainQuadNode::worldRect() const { return Rect3Df(localRect(), terrain->transform()); }

Rect2Df TerrainQuadNode::localRect() const {
    glm::vec2 bl(local.x - size / 2.f, local.y - size / 2.f);
    glm::vec2 tr(local.x + size / 2.f, local.y + size / 2.f);
    return Rect2Df(bl, tr);
}
