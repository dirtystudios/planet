#pragma once
#include <stdint.h>

namespace graphics {
enum class FillMode : uint8_t {
    Wireframe = 0,
    Solid,
    Count,
};
}
