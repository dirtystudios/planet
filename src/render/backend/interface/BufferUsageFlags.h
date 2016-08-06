#pragma once

#include "EnumTraits.h"
#include <array>

namespace gfx {
enum class BufferUsageFlags : uint16_t {
    None              = 0,
    VertexBufferBit   = 1 << 0,
    IndexBufferBit    = 1 << 1,
    ConstantBufferBit = 1 << 2,
};
}

template <>
struct EnumTraits<gfx::BufferUsageFlags> {
    static constexpr bool is_bitflags = true;
    static constexpr size_t count     = 4;
};

namespace BufferUsageFlagsHelpers {
static constexpr std::array<gfx::BufferUsageFlags, EnumTraits<gfx::BufferUsageFlags>::count> values{
    {gfx::BufferUsageFlags::None, gfx::BufferUsageFlags::VertexBufferBit, gfx::BufferUsageFlags::IndexBufferBit,
     gfx::BufferUsageFlags::ConstantBufferBit}};
}
