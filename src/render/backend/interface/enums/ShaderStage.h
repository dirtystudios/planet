#pragma once

#include <stdint.h>
#include <functional>

namespace gfx {
enum class ShaderStage : uint8_t {
    Vertex = 0,
    TessControl,
    TessEval,
    Pixel,
    Count,
};
}

namespace std {
template <> struct hash<gfx::ShaderStage> {
    size_t operator()(const gfx::ShaderStage& x) const {
        return std::hash<std::underlying_type<gfx::ShaderStage>::type>()(
            static_cast<std::underlying_type<gfx::ShaderStage>::type>(x));
    }
};
}
