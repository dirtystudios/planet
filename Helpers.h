#ifndef __helpers_h__
#define __helpers_h__

#include <glm/glm.hpp>
#include <sstream>

const glm::vec3 X_AXIS = glm::vec3(1, 0, 0);
const glm::vec3 Y_AXIS = glm::vec3(0, 1, 0);
const glm::vec3 Z_AXIS = glm::vec3(0, 0, 1);
const float PI = 3.1415926f;

static void Orthogonalize(glm::vec3* v1, glm::vec3* v2, glm::vec3* v3) {
    *v1 = glm::normalize(*v1);
    *v2 = glm::normalize(glm::cross(*v3, *v1));
    *v3 = glm::cross(*v1, *v2);
}

static std::ostream& operator<<(std::ostream& os, const glm::vec3& v) {
    return os << v.x << ", " << v.y << ", " << v.z;
}

static inline std::string ToString(const glm::vec3& v) {
    std::stringstream ss;
    ss << v;
    return ss.str();
}

static inline float ToRadians(float degrees) {
    return degrees * PI / 180.f;
}

template <class T>
static inline void HashCombine(std::size_t& seed, const T& v) {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

#endif