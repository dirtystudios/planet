#pragma once

#include <stdint.h>

namespace gfx {
enum class BlendMode : uint8_t {
    Add = 0,
    Subtract,
    ReverseSubtract,
    Min,
    Max,
    Count,
};
}
