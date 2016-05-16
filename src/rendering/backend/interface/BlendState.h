#pragma once

#include "BlendFunc.h"
#include "BlendMode.h"
#include "Helpers.h"

namespace graphics {
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

namespace std
{
    template<> struct hash<graphics::BlendState>
    {
        typedef graphics::BlendState argument_type;
        typedef std::size_t result_type;
        result_type operator()(argument_type const& s) const
        {
            result_type key = 0;
            HashCombine(key, s.enable);
            HashCombine(key, (uint32_t)s.srcRgbFunc);
            HashCombine(key, (uint32_t)s.srcAlphaFunc);
            HashCombine(key, (uint32_t)s.dstRgbFunc);
            HashCombine(key, (uint32_t)s.dstAlphaFunc);
            HashCombine(key, (uint32_t)s.rgbMode);
            HashCombine(key, (uint32_t)s.alphaMode);
            return key;
        }
    };
}