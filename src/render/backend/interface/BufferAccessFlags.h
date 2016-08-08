#pragma once

#include "EnumTraits.h"

namespace gfx {
enum class BufferAccessFlags : uint16_t {
    None        = 0,
    GpuReadBit  = 1 << 0,
    GpuWriteBit = 1 << 1,
    CpuReadBit  = 1 << 2,
    CpuWriteBit = 1 << 3,

    GpuReadCpuWriteBits = GpuReadBit | CpuWriteBit,
};
}

template <>
struct is_enum_flags<gfx::BufferAccessFlags> : public std::true_type {};

