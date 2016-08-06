#pragma once

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
struct EnumTraits<gfx::BufferAccessFlags> {
    static constexpr bool is_bitflags = true;
    static constexpr size_t count     = 4;
};
