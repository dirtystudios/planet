
#include "Frustum.h"
#include "glm/gtc/matrix_access.hpp"

Frustum::Frustum(const glm::mat4& projection, const glm::mat4& view) {
    glm::mat4 view_proj = projection * view;

    frustum_planes[0] = glm::row(view_proj, 3) + glm::row(view_proj, 0); // left
    frustum_planes[1] = glm::row(view_proj, 3) - glm::row(view_proj, 0); // right
    frustum_planes[2] = glm::row(view_proj, 3) + glm::row(view_proj, 1); // bottom
    frustum_planes[3] = glm::row(view_proj, 3) - glm::row(view_proj, 1); // top
    frustum_planes[4] = glm::row(view_proj, 3) + glm::row(view_proj, 2); // far
    frustum_planes[5] = glm::row(view_proj, 3) - glm::row(view_proj, 2); // near
    
    for(int i = 0; i < 6; i++) {
        float len = glm::length(glm::vec3(frustum_planes[i].x, frustum_planes[i].y, frustum_planes[i].z));       
        frustum_planes[i] = frustum_planes[i] / len;        
    }    
}
    
bool Frustum::IsPointInFrustum(const glm::vec3& p) {
    for(int i = 0; i < 6; ++i) {
        if(glm::dot(glm::vec4(p.x, p.y, p.z, 1.f), frustum_planes[i]) < 0) {
            return false;                    
        }
    }
    return true;
}

bool Frustum::IsBoxInFrustum(const BoundingBox& box) {
    return
    IsPointInFrustum(box.min) ||
    IsPointInFrustum(box.max) ||
    IsPointInFrustum(glm::vec3(box.min.x, box.min.y, box.max.z)) ||
    IsPointInFrustum(glm::vec3(box.min.x, box.max.y, box.min.z)) ||
    IsPointInFrustum(glm::vec3(box.max.x, box.min.y, box.min.z)) ||
    IsPointInFrustum(glm::vec3(box.max.x, box.max.y, box.min.z)) ||
    IsPointInFrustum(glm::vec3(box.max.x, box.min.y, box.max.z)) ||
    IsPointInFrustum(glm::vec3(box.min.x, box.max.y, box.max.z));

}




