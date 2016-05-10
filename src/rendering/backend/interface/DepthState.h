#pragma once

#include "DepthWriteMask.h"
#include "DepthFunc.h"

namespace graphics {
struct DepthState {
    bool enable{true};
    DepthWriteMask depthWriteMask{DepthWriteMask::All};
    DepthFunc depthFunc{DepthFunc::Less};
};
}