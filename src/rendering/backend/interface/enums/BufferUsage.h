#pragma once

#include <stdint.h>

namespace graphics {
enum class BufferUsage : uint32_t {
    Static = 0,
    Dynamic,
    Count,
};
}