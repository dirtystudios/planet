#pragma once

#include <stdint.h>

namespace graphics {
enum class TextureSlot : uint8_t {
    Base = 0,
    Normal,
    Count,
};
}