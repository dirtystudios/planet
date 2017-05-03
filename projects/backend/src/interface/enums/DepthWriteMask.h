#pragma once
#include <stdint.h>

namespace gfx {
enum class DepthWriteMask : uint8_t {
    Zero = 0,
    All,
    Count,
};
}
