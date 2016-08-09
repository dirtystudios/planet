#pragma once

#include "EnumTraits.h"

namespace gfx {
enum class ShaderStageFlags : uint8_t {
    None           = 0,
    VertexBit      = 1 << 0,
    TessControlBit = 1 << 1,
    TessEvalBit    = 1 << 2,
    ComputeBit     = 1 << 3,
    PixelBit       = 1 << 4,

    AllStages = 0x0f
};
}

template <>
struct is_enum_flags<gfx::ShaderStageFlags> : public std::true_type {};
