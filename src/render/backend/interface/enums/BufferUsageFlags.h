#pragma once

#include "EnumTraits.h"

namespace gfx {
enum class BufferUsageFlags : uint16_t {
    None              = 0,
    VertexBufferBit   = 1 << 0,
    IndexBufferBit    = 1 << 1,
    ConstantBufferBit = 1 << 2,
};
}
ALLOW_FLAGS_FOR_ENUM(gfx::BufferUsageFlags);
