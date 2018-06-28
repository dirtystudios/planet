//
//  Framebuffer.h
//  planet
//
//  Created by Eugene Sturm on 6/22/18.
//

#pragma once

#include "ResourceTypes.h"

namespace gfx
{
    
    struct FrameBuffer
    {
        TextureId color[8] { 0 };
        uint32_t colorCount { 0 };
        TextureId depth { 0 };
    };
}
