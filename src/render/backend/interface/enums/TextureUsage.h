//
//  TextureUsage.h
//  planet
//
//  Created by Eugene Sturm on 6/22/18.
//

#pragma once
#include "EnumTraits.h"

namespace gfx
{
    enum class TextureUsageFlags : uint8_t {
        None = 0,
        ShaderRead = 1 << 1,
        ShaderWrite = 1 << 2,
        RenderTarget = 1 << 3,

        ShaderRW = ShaderRead | ShaderWrite,
    };
}
ALLOW_FLAGS_FOR_ENUM(gfx::TextureUsageFlags);
