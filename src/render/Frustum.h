#ifndef __frustum_h__
#define __frustum_h__

#include "BoundingBox.h"

struct Frustum {
    Frustum(const glm::mat4& projection, const glm::mat4& view);

    glm::dvec4 frustum_planes[6];

    bool IsPointInFrustum(const glm::dvec3& p) const;
    bool IsBoxInFrustum(const BoundingBox& bbox) const;
};


#endif
