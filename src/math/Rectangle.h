#pragma once

#include <array>
#include <glm/glm.hpp>
#include <sstream>
#include <string>
#include "DMath.h"
#include "Helpers.h"

namespace dm {

template <typename T>
class Rect2D {
private:
    using vec2 = glm::tvec2<T, glm::precision::highp>;
    vec2 _corners[4]{};

public:
    Rect2D(const vec2& dimensions) : Rect2D(vec2(-dimensions.x / T(2), -dimensions.y / T(2)), vec2(dimensions.x / T(2), dimensions.y / T(2))) {}
    Rect2D(const vec2& bl, const vec2& tr) {
        _corners[0] = bl;
        _corners[1] = vec2(tr.x, bl.y);
        _corners[2] = vec2(bl.x, tr.y);
        _corners[3] = tr;
    }

    float width() const { return std::abs(tr().x - tl().x); }
    float height() const { return std::abs(tr().y - br().y); }

    std::array<Rect2D<T>, 4> subdivide() const {
        vec2 middle = center();
        return {{
            Rect2D<T>(bl(), middle),                                         // bl rect
            Rect2D<T>(dm::lerp(bl(), br(), 0.5), dm::lerp(br(), tr(), 0.5)), // br rect
            Rect2D<T>(dm::lerp(bl(), tl(), 0.5), dm::lerp(tl(), tr(), 0.5)), // tl rect
            Rect2D<T>(middle, tr()),                                         // tr rect

        }};
    }

    const vec2& bl() const { return _corners[0]; };
    const vec2& br() const { return _corners[1]; };
    const vec2& tr() const { return _corners[3]; };
    const vec2& tl() const { return _corners[2]; };
    const vec2& center() const { return lerp(bl(), tr(), 0.5); }
};

using Rect2Df = Rect2D<float>;
using Rect2Dd = Rect2D<double>;

template <typename T>
class Rect3D {
private:
    template <typename P>
    using copy_vec3 = glm::tvec3<P, glm::precision::highp>;
    using vec3      = glm::tvec3<T, glm::precision::highp>;
    using vec2      = glm::tvec2<T, glm::precision::highp>;
    using mat4      = glm::tmat4x4<T, glm::precision::highp>;
    std::array<vec3, 4> _corners{};

public:
    Rect3D(const vec2& dimensions, const glm::mat4& transform = glm::mat4())
        : Rect3D(Rect2D<T>(dimensions), transform) {}

    Rect3D(const vec3& bl, const vec3& br, const vec3& tl, const vec3& tr)
        : _corners({{bl, br, tl, tr}}) {}

    Rect3D(const Rect3D<T>& rect, const glm::mat4& transform) {
        _corners[0] = glm::vec3(transform * glm::vec4(rect.bl().x, rect.bl().y, rect.bl().z, 1.f));
        _corners[1] = glm::vec3(transform * glm::vec4(rect.br().x, rect.br().y, rect.br().z, 1.f));
        _corners[2] = glm::vec3(transform * glm::vec4(rect.tl().x, rect.tl().y, rect.tl().z, 1.f));
        _corners[3] = glm::vec3(transform * glm::vec4(rect.tr().x, rect.tr().y, rect.tr().z, 1.f));
    }
    Rect3D(const Rect2D<T>& rect, const glm::mat4& transform = glm::mat4()) {
        _corners[0] = glm::vec3(transform * glm::vec4(rect.bl().x, rect.bl().y, 0.f, 1.f));
        _corners[1] = glm::vec3(transform * glm::vec4(rect.br().x, rect.br().y, 0.f, 1.f));
        _corners[2] = glm::vec3(transform * glm::vec4(rect.tl().x, rect.tl().y, 0.f, 1.f));
        _corners[3] = glm::vec3(transform * glm::vec4(rect.tr().x, rect.tr().y, 0.f, 1.f));
    }

    template <typename P>
    Rect3D(const Rect3D<P>& copy) : Rect3D(copy_vec3<P>(copy.bl()), copy_vec3<P>(copy.br()), copy_vec3<P>(copy.tl()), copy_vec3<P>(copy.tr())) {}

    std::array<Rect3D<T>, 4> subdivide() const {
        vec3 middle      = center();
        vec3 middleTop   = dm::lerp(tl(), tr(), 0.5);
        vec3 middleLeft  = dm::lerp(bl(), tl(), 0.5);
        vec3 middleRight = dm::lerp(br(), tr(), 0.5);
        vec3 middleBot   = dm::lerp(bl(), br(), 0.5);

        return {{
            Rect3D<T>(bl(), middleBot, middleLeft, middle),  // bl rect
            Rect3D<T>(middleBot, br(), middle, middleRight), // br rect
            Rect3D<T>(middleLeft, middle, tl(), middleTop),  // tl rect
            Rect3D<T>(middle, middleRight, middleTop, tr()), // tr rect
        }};
    }

    const vec3& bl() const { return _corners[0]; };
    const vec3& br() const { return _corners[1]; };
    const vec3& tl() const { return _corners[2]; };
    const vec3& tr() const { return _corners[3]; };

    T    width() const { return std::abs(bl().x - br().x); }
    T    height() const { return std::abs(tl().y - bl().y); }
    vec3 center() const { return lerp(bl(), tr(), 0.5f); }

    std::array<vec3, 4> corners() const { return _corners; }

    void transform(const glm::mat4& transform) {
        for (vec3& corner : _corners) {
            glm::vec4 transformedCorner = transform * glm::vec4(corner.x, corner.y, corner.z, 1);
            corner                      = glm::vec3(transformedCorner);
        }
    }

    void scale(const vec3& scaleFactors) {
        mat4 transform = glm::scale(mat4(), scaleFactors);
        for (vec3& corner : _corners) {
            glm::vec4 transformedCorner = transform * glm::vec4(corner.x, corner.y, corner.z, 1);
            corner                      = glm::vec3(transformedCorner);
        }
    }
};

using Rect3Df = Rect3D<float>;
using Rect3Dd = Rect3D<double>;

template <typename T>
static std::string ToString(const Rect3D<T>& rect) {
    std::stringstream ss;
    ss << "Rect[bl:" << rect.bl() << " br:" << rect.br() << " tl:" << rect.tl() << " tr:" << rect.tr() << "]";
    return ss.str();
}
}
