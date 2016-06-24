#pragma once

#include <stdint.h>

namespace gfx {
enum class BufferUsage : uint32_t {
    Static = 0,
    Dynamic,
    Count,
};
}
