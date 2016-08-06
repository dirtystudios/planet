#pragma once

#include "FillMode.h"
#include "CullMode.h"
#include "WindingOrder.h"
#include "Hash.h"

namespace gfx {
struct RasterState {
    FillMode fillMode{FillMode::Solid};
    CullMode cullMode{CullMode::Back};
    WindingOrder windingOrder{WindingOrder::FrontCCW};
};
}
namespace std {
template <>
struct hash<gfx::RasterState> {
    typedef gfx::RasterState argument_type;
    typedef std::size_t result_type;
    result_type operator()(argument_type const& s) const { return HashCombine(s.fillMode, s.cullMode, s.windingOrder); }
};
}
