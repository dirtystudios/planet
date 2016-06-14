#pragma once
#include <stdint.h>

namespace graphics {
enum class DepthFunc : uint8_t {
    Never = 0,
    Less,
    Equal,
    LessEqual,
    Greater,
    NotEqual,
    GreaterEqual,
    Always,
    Count,
};
}