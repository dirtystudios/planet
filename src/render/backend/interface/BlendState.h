#pragma once

#include "Hash.h"
#include "BlendFunc.h"
#include "BlendMode.h"

namespace gfx {
struct BlendState {
    bool enable{false};
    BlendFunc srcRgbFunc{BlendFunc::One};
    BlendFunc srcAlphaFunc{BlendFunc::Zero};
    BlendFunc dstRgbFunc{BlendFunc::One};
    BlendFunc dstAlphaFunc{BlendFunc::Zero};
    BlendMode rgbMode{BlendMode::Add};
    BlendMode alphaMode{BlendMode::Add};
};
}

namespace std {
template <>
struct hash<gfx::BlendState> {
    typedef gfx::BlendState argument_type;
    typedef std::size_t result_type;
    result_type operator()(argument_type const& s) const {
        return HashCombine(s.enable, s.srcRgbFunc, s.srcAlphaFunc, s.dstRgbFunc, s.dstAlphaFunc, s.rgbMode,
                           s.alphaMode);
    }
};
}
