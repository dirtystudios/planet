#pragma once

#include "DepthWriteMask.h"
#include "DepthFunc.h"


namespace gfx {
struct DepthState {
    bool enable{true};
    DepthWriteMask depthWriteMask{DepthWriteMask::All};
    DepthFunc depthFunc{DepthFunc::Less};
};
}

namespace std
{
    template<> struct hash<gfx::DepthState>
    {
        typedef gfx::DepthState argument_type;
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
