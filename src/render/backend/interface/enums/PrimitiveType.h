#pragma once

#include <stdint.h>
namespace gfx {
enum class PrimitiveType : uint8_t {
    Triangles = 0,
    LineStrip = 1,
    Lines,
    Count,
};
}
namespace std {
template <> struct hash<gfx::PrimitiveType> {
    size_t operator()(const gfx::PrimitiveType& x) const {
        return std::hash<std::underlying_type<gfx::PrimitiveType>::type>()(
            static_cast<std::underlying_type<gfx::PrimitiveType>::type>(x));
    }
};
}
