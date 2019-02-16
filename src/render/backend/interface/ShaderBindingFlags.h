#pragma once

#include <cstdint>

namespace gfx {
    enum class ShaderBindingFlags : uint8_t {
        SampleRead = 0,
        ReadOnly   = 1,
        ReadWrite  = 2,
    };
}