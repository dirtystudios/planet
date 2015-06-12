#ifndef __frustum_h__
#define __frustum_h__

#include <glm/glm.hpp>
#include "BoundingBox.h"

struct Frustum {
    Frustum(const glm::mat4& projection, const glm::mat4& view);

    glm::vec4 frustum_planes[6];

    bool IsPointInFrustum(const glm::vec3& p);    
    bool IsBoxInFrustum(const BoundingBox& bbox);
};


#endif
