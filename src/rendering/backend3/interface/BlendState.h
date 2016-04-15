#pragma once

#include "BlendFunc.h"
#include "BlendMode.h"

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