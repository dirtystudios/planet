#pragma once

#include <stdint.h>

namespace graphics {
enum class TextureFormat : uint8_t {
    R32F = 0,
    RGB32F,
    RGBA32F,
    R_U8,
    RGB_U8,
    RGBA_U8,
    Count,
};
}