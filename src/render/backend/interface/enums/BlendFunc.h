#pragma once

#include <functional>

namespace gfx {
enum class BlendFunc : uint8_t {
    Zero = 0,
    One,
    SrcColor,
    OneMinusSrcColor,
    SrcAlpha,
    OneMinusSrcAlpha,
    DstAlpha,
    OneMinusDstAlpha,
    ConstantColor,
    OneMinusConstantColor,
    ConstantAlpha,
    OneMinusConstantAlpha,
    SrcAlphaSaturate,
    Src1Color,
    OneMinusSrc1Color,
    Src1Alpha,
    OneMinusSrc1Alpha,
    Count,
};
}

namespace std {
template <> struct hash<gfx::BlendFunc> {
    size_t operator()(const gfx::BlendFunc& x) const {
        return std::hash<std::underlying_type<gfx::BlendFunc>::type>()(
            static_cast<std::underlying_type<gfx::BlendFunc>::type>(x));
    }
};
}
