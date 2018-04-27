#pragma once

#include <stdint.h>

namespace gfx {

enum class PixelFormat : uint8_t {
    R8Unorm = 0,
    RGB8Unorm,
    RGBA8Unorm,

    R8Uint,

    R32Float,
    RGB32Float,
    RGBA32Float,
    BGRA8Unorm,
    Count,
};
}
