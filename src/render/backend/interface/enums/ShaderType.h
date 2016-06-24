#pragma once

#include <stdint.h>
#include <functional>

namespace gfx {
enum class ShaderType : uint8_t {
    VertexShader = 0,
    TessControlShader,
    TessEvalShader,
    PixelShader,
    Count,
};
}

namespace std {
template <> struct hash<gfx::ShaderType> {
    size_t operator()(const gfx::ShaderType& x) const {
        return std::hash<std::underlying_type<gfx::ShaderType>::type>()(
            static_cast<std::underlying_type<gfx::ShaderType>::type>(x));
    }
};
}
