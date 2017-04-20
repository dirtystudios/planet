
#include "Frustum.h"
#include "glm/gtc/matrix_access.hpp"

Frustum::Frustum(const glm::mat4& projection, const glm::mat4& view) {
    glm::mat4 view_proj = projection * view;

    std::array<glm::dvec4, 6> p;

    // planes with their normals pointing to the inside of the frustum
    p[0] = glm::row(view_proj, 3) + glm::row(view_proj, 0); // left
    p[1] = glm::row(view_proj, 3) - glm::row(view_proj, 0); // right
    p[2] = glm::row(view_proj, 3) + glm::row(view_proj, 1); // bottom
    p[3] = glm::row(view_proj, 3) - glm::row(view_proj, 1); // top
    p[4] = glm::row(view_proj, 3) + glm::row(view_proj, 2); // far
    p[5] = glm::row(view_proj, 3) - glm::row(view_proj, 2); // near

    for (int i = 0; i < 6; i++) {
        double len = glm::length(glm::vec3(p[i].x, p[i].y, p[i].z));
        _planes[i] = dm::Plane3Dd(p[i] / len);
    }
}

bool Frustum::IsPointInFrustum(const glm::dvec3& p) const {
    for (int i = 0; i < 6; ++i) {
        if (_planes[i].inFront(p)) {
            return true;
        }
    }
    return false;
}

bool Frustum::IsBoxInFrustum(const dm::BoundingBox& box) const {
    std::array<glm::vec3, 8> corners = box.corners();

    // check that all corners of the box are on the internal side of every plane
    for (int i = 0; i < 6; ++i) {

        bool areAllExternal = true;
        for (glm::vec3& p : corners) {
            if (_planes[i].inFront(p)) {
                areAllExternal = false;
                break;
            }
        }

        // if all points are external to one plane, it can't be in the frustum
        if (areAllExternal) {
            return false;
        }
    }

    // TODO: test against frustum corners
    return true;
}
