#pragma once

#include "FillMode.h"
#include "CullMode.h"
#include "WindingOrder.h"
#include "Helpers.h"

namespace gfx {
struct RasterState {
    FillMode fillMode{FillMode::Solid};
    CullMode cullMode{CullMode::Back};
    WindingOrder windingOrder{WindingOrder::FrontCCW};
};
}
namespace std
{
    template<> struct hash<gfx::RasterState>
    {
        typedef gfx::RasterState argument_type;
        typedef std::size_t result_type;
        result_type operator()(argument_type const& s) const
        {
            result_type key = 0;
            HashCombine(key, (uint32_t)s.fillMode);
            HashCombine(key, (uint32_t)s.cullMode);
            HashCombine(key, (uint32_t)s.windingOrder);
            return key;
        }
    };
}
