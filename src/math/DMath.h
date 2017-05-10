#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <type_traits>

namespace dm {

// constexpr pow function
// http://stackoverflow.com/a/16443849
template <class T>
constexpr T pow(const T base, unsigned const exponent) {
    return (exponent == 0) ? 1 : (base * pow(base, exponent - 1));
}
template <typename T, T base, unsigned exponent>
using pow_ = std::integral_constant<T, pow(base, exponent)>;

constexpr float kPI               = 3.1415926f;
constexpr float kRadiansPerDegree = kPI / 180.f;

using Degrees = float;
using Radians = float;

constexpr Radians toRadians(Degrees degrees) { return degrees * kRadiansPerDegree; }
constexpr Degrees toDegrees(Radians radians) { return radians / kRadiansPerDegree; }

class Transform {
private:
    glm::mat4 _rotation;
    glm::mat4 _scale;
    glm::mat4 _translation;

public:
    Transform() {}

    void rotateDegrees(Degrees degrees, const glm::vec3& axis) { rotateRadians(toRadians(degrees), axis); }
    void rotateRadians(Radians radians, const glm::vec3& axis) { _rotation = glm::rotate(_rotation, radians, axis); }
    void scale(const glm::vec3& factors) { _scale = glm::scale(_scale, factors); }
    void scale(float scalar) { scale({scalar, scalar, scalar}); };
    void translate(const glm::vec3& translation) { _translation = glm::translate(_translation, translation); }

    void reset() {
        _scale = glm::mat4();
        _rotation = glm::mat4();
        _translation = glm::mat4();
    }
    
    glm::mat4 matrix() { return _translation * _rotation * _scale; }
};

template <typename T>
constexpr T lerp(const T& a, const T& b, double t) {
    return ((1.f - t) * a) + (t * b);
}

template <typename T>
constexpr T clamp(const T& val, const T& min, const T& max) {
    return val > max ? max : (val < min ? min : val);
}
}
