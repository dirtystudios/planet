//
//  Swapchain.h
//  planet
//
//  Created by Eugene Sturm on 6/22/18.
//

#pragma once

#include "ResourceTypes.h"
#include "PixelFormat.h"

namespace gfx
{
    struct SwapchainDesc
    {
        PixelFormat format;
        uint32_t width;
        uint32_t height;
    };
    
    class Swapchain
    {
    public:
        virtual TextureId begin() = 0;
        virtual void present(TextureId surface) = 0;
        virtual PixelFormat pixelFormat() const = 0;
        virtual uint32_t width() const = 0;
        virtual uint32_t height() const = 0;
    };
}
