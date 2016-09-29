#pragma once

#include <glm/glm.hpp>
#include <sstream>
#include <string>
#include "Helpers.h"

namespace dm {

template <typename T>
class Rect2D {
private:
    using vec2 = glm::tvec2<T, glm::precision::highp>;
    vec2 _corners[4]{};

public:
    Rect2D(const vec2& bl, const vec2& tr) {
        _corners[0] = bl;
        _corners[1] = vec2(tr.x, bl.y);
        _corners[2] = tr;
        _corners[3] = vec2(bl.x, tr.y);
    }

    float width() const { return std::abs(tr().x - tl().x); }
    float height() const { return std::abs(tr().y - br().y); }

    const vec2& bl() const { return _corners[0]; };
    const vec2& br() const { return _corners[1]; };
    const vec2& tr() const { return _corners[2]; };
    const vec2& tl() const { return _corners[3]; };
};

using Rect2Df = Rect2D<float>;

template <typename T>
class Rect3D {
private:
    using vec3 = glm::tvec3<T, glm::precision::highp>;
    vec3 _corners[4]{};

public:
    Rect3D(const Rect2D<T>& rect, const glm::mat4& transform = glm::mat4()) {
        _corners[0] = glm::vec3(transform * glm::vec4(rect.bl().x, rect.bl().y, 0.f, 1.f));
        _corners[1] = glm::vec3(transform * glm::vec4(rect.br().x, rect.br().y, 0.f, 1.f));
        _corners[2] = glm::vec3(transform * glm::vec4(rect.tr().x, rect.tr().y, 0.f, 1.f));
        _corners[3] = glm::vec3(transform * glm::vec4(rect.tl().x, rect.tl().y, 0.f, 1.f));
    }

    const vec3& bl() const { return _corners[0]; };
    const vec3& br() const { return _corners[1]; };
    const vec3& tr() const { return _corners[2]; };
    const vec3& tl() const { return _corners[3]; };
};

using Rect3Df = Rect3D<float>;

static std::string ToString(const Rect3D<float>& rect) {
    std::stringstream ss;
    ss << "Rect[bl:" << rect.bl() << " br:" << rect.br() << " tr:" << rect.tr() << " tr:" << rect.tr() << "]";
    return ss.str();
}
}
