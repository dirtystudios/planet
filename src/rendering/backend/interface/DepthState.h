#pragma once

#include "DepthWriteMask.h"
#include "DepthFunc.h"
#include "Helpers.h"

namespace graphics {
struct DepthState {
    bool enable{true};
    DepthWriteMask depthWriteMask{DepthWriteMask::All};
    DepthFunc depthFunc{DepthFunc::Less};
};
}

namespace std
{
    template<> struct hash<graphics::DepthState>
    {
        typedef graphics::DepthState argument_type;
        typedef std::size_t result_type;
        result_type operator()(argument_type const& s) const
        {
            result_type key = 0;
            HashCombine(key, s.enable);
            HashCombine(key, (uint32_t)s.depthWriteMask);
            HashCombine(key, (uint32_t)s.depthFunc);
            return key;
        }
    };
}