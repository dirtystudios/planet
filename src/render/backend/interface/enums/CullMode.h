#pragma once
#include <stdint.h>

namespace gfx {
enum class CullMode : uint8_t {
    None = 0,
    Front,
    Back,
    Count,
};
}
