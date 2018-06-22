#pragma once

#include <stdint.h>

namespace gfx {

enum class PixelFormat : uint8_t {
    Invalid = 0,
    R8Unorm,
    RGB8Unorm,
    RGBA8Unorm,
    R8Uint,
    R32Float,
    RGB32Float,
    RGBA32Float,
    BGRA8Unorm,
    Depth32Float,
    Depth24FloatStencil8,
    Count,
};
}
