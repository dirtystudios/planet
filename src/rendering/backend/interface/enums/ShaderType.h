#pragma once

#include <stdint.h>
#include <functional>

namespace graphics {
enum class ShaderType : uint8_t {
    VertexShader = 0,
    TessControlShader,
    TessEvalShader,
    PixelShader,
    Count,
};
}

namespace std {
template <> struct hash<graphics::ShaderType> {
    size_t operator()(const graphics::ShaderType& x) const {
        return std::hash<std::underlying_type<graphics::ShaderType>::type>()(
            static_cast<std::underlying_type<graphics::ShaderType>::type>(x));
    }
};
}