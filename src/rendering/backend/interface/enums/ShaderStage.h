#pragma once

#include <stdint.h>
#include <functional>

namespace graphics {
enum class ShaderStage : uint8_t {
    Vertex = 0,
    TessControl,
    TessEval,
    Pixel,
    Count,
};
}

namespace std {
template <> struct hash<graphics::ShaderStage> {
    size_t operator()(const graphics::ShaderStage& x) const {
        return std::hash<std::underlying_type<graphics::ShaderStage>::type>()(
            static_cast<std::underlying_type<graphics::ShaderStage>::type>(x));
    }
};
}