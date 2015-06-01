#include "Frustum.h"
#include <glm/gtc/matrix_access.hpp>

Frustum::Frustum(const glm::mat4& projection, const glm::mat4& view) {
    ComputeFrustumPlanes(projection, view);
}

Frustum::Frustum() {    
}

    
bool Frustum::ContainsPoint(const glm::vec3& p) {
    for(int i = 0; i < 6; ++i) {
        if(glm::dot(glm::vec4(p.x, p.y, p.z, 1.f), _frustum_planes[i]) < 0) {
            return false;                    
        }
    }
    return true;
}


void Frustum::ComputeFrustumPlanes(const glm::mat4& projection, const glm::mat4& view) {    
    glm::mat4 view_proj = projection * view;

    _frustum_planes[0] = glm::row(view_proj, 3) + glm::row(view_proj, 0); // left
    _frustum_planes[1] = glm::row(view_proj, 3) - glm::row(view_proj, 0); // right
    _frustum_planes[2] = glm::row(view_proj, 3) + glm::row(view_proj, 1); // bottom
    _frustum_planes[3] = glm::row(view_proj, 3) - glm::row(view_proj, 1); // top
    _frustum_planes[4] = glm::row(view_proj, 3) + glm::row(view_proj, 2); // far
    _frustum_planes[5] = glm::row(view_proj, 3) - glm::row(view_proj, 2); // near
    
    for(int i = 0; i < 6; i++) {
        float len = glm::length(glm::vec3(_frustum_planes[i].x, _frustum_planes[i].y, _frustum_planes[i].z));       
        _frustum_planes[i] = _frustum_planes[i] / len;        
    }    
}

const glm::vec4* Frustum::GetPlanes() {
    return _frustum_planes;
}