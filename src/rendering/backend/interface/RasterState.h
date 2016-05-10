#pragma once

#include "FillMode.h"
#include "CullMode.h"
#include "WindingOrder.h"

namespace graphics {
struct RasterState {
    FillMode fillMode{FillMode::Solid};
    CullMode cullMode{CullMode::Back};
    WindingOrder windingOrder{WindingOrder::FrontCCW};
};
}