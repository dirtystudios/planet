#pragma once

#include "EnumTraits.h"

namespace gfx {
enum class BufferUsageFlags : uint16_t {
    None              = 0,
    VertexBufferBit   = 1 << 0,
    IndexBufferBit    = 1 << 1,
    ConstantBufferBit = 1 << 2,
    UnorderedAccessViewBit = 1 << 3,
};
}

template <>
struct is_enum_flags<gfx::BufferUsageFlags> : public std::true_type {};
