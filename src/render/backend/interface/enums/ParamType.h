#pragma once

#include <stdint.h>
#include <cstdlib>
#include <cassert>
#include <functional>

namespace gfx {
enum class ParamType : uint8_t {
    Int32 = 0,
    UInt32,
    Float,
    Float2,
    Float3,
    Float4,
    Float4x4,
    Count,
};

static size_t GetByteCount(ParamType paramType) {
    switch (paramType) {
    case ParamType::Float:
        return sizeof(float);
    case ParamType::UInt32:
        return sizeof(uint32_t);
    case ParamType::Int32:
        return sizeof(int32_t);
    case ParamType::Float2:
        return sizeof(float) * 2;
    case ParamType::Float3:
        return sizeof(float) * 3;
    case ParamType::Float4:
        return sizeof(float) * 4;
    case ParamType::Float4x4:
        return sizeof(float) * 16;
    default:
        assert(false);
        return 0;
    }
}
}

namespace std {
template <> struct hash<gfx::ParamType> {
    size_t operator()(const gfx::ParamType& x) const {
        return std::hash<std::underlying_type<gfx::ParamType>::type>()(
            static_cast<std::underlying_type<gfx::ParamType>::type>(x));
    }
};
}
