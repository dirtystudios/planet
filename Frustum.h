#ifndef __frustum_h__
#define __frustum_h__
#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>

class Frustum {
private:
    glm::vec4 _frustum_planes[6];
public:
    Frustum(const glm::mat4& projection, const glm::mat4& view);
    Frustum();

    bool ContainsPoint(const glm::vec3& p);
    void ComputeFrustumPlanes(const glm::mat4& projection, const glm::mat4& view);
    const glm::vec4* GetPlanes();
};

#endif