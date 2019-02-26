#ifndef __frustum_h__
#define __frustum_h__

#include "BoundingBox.h"
#include "Plane.h"

class Frustum {
private:
    std::array<dm::Plane3Dd, 6> _planes;

public:
    Frustum() = default;
    Frustum(const glm::mat4& projection, const glm::mat4& view);

    bool IsPointInFrustum(const glm::dvec3& p) const;
    bool IsBoxInFrustum(const dm::BoundingBox& bbox) const;
};

#endif
