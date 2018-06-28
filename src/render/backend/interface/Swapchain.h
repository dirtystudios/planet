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
    private:
        SwapchainDesc _desc;
    public:
        Swapchain(const SwapchainDesc& desc) : _desc(desc) {}
        
        virtual TextureId begin() = 0;
        virtual void present(TextureId surface) = 0;
        
        PixelFormat pixelFormat() const { return _desc.format; };
        uint32_t width() const { return _desc.width; }
        uint32_t height() const { return _desc.height; }
        
        void resize(uint32_t width, uint32_t height)
        {
            onSwapchainResize(width, height);
            _desc.width = width;
            _desc.height = height;
        }
    protected:
        virtual void onSwapchainResize(uint32_t width, uint32_t height) = 0;
    };
}
