#pragma once

namespace graphics {
enum class CullMode : uint8_t {
    None = 0,
    Front,
    Back,
    Count,
};
}