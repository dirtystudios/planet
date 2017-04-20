#pragma once

#include <glm/glm.hpp>

namespace dm {

template <typename T>
class Plane3D {
private:
    using vec3 = glm::tvec3<T, glm::precision::highp>;
    using vec4 = glm::tvec4<T, glm::precision::highp>;
    vec4 _p;

public:
    Plane3D() {}

    Plane3D(const vec4& params) : _p(params) {}

    Plane3D(T a, T b, T c, T d) : _p(vec4(a, b, c, d)) {}

    Plane3D(const vec3& n, const vec3& p) : _p(vec4(n.a, n.b, n.c, glm::dot(-p, n))) {}

    T distance(const vec3& p) const { return glm::dot(vec4(p.x, p.y, p.z, 1.f), _p); }

    bool inFront(const vec3& p) const { return distance(p) > 0; }
};

using Plane3Dd = Plane3D<double>;
}
