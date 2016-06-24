#pragma once

#include <stdint.h>

namespace gfx {
enum class TextureSlot : uint8_t {
    Base = 0,
    Normal,
    Count,
};
}
