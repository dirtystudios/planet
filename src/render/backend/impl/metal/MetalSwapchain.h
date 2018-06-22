//
//  MetalSwapchain.hpp
//  planet
//
//  Created by Eugene Sturm on 4/12/18.
//

#ifndef MetalSwapchain_hpp
#define MetalSwapchain_hpp

#import "Swapchain.h"
#import <Metal/Metal.h>
#import "MetalResources.h"
#import <QuartzCore/CAMetalLayer.h>
#include <array>

namespace gfx
{
    class ResourceManager;
    class MetalSwapchain;
    
    class MetalSwapchainImage : public MetalTexture
    {
    private:
        MetalSwapchain* _swapchain { nullptr };
    public:
        MetalSwapchainImage(MetalSwapchain* swapchain)
        : _swapchain(swapchain)
        {}
        
        id<CAMetalDrawable> drawable { nil };
    };
    
    class MetalSwapchain : public Swapchain
    {
    private:
        // Metal supports 3 concurrent drawables, but if the
        // swapchain is destroyed and rebuilt as part of resizing,
        // one will be held by the current display
        static constexpr uint32_t kImageCount = 2;
        
        dispatch_semaphore_t _inflightSemaphore;
        CAMetalLayer* _metalLayer;
        id<CAMetalDrawable> _currentDrawable;
        id<MTLCommandQueue> _commandQueue;
        ResourceManager* _resourceManager;
        std::array<MetalSwapchainImage*, kImageCount> _swapchainImages;
        uint32_t _imageIdx {0};
    public:
        MetalSwapchain(id<MTLCommandQueue> commandQueue, ResourceManager* resourceManager, CAMetalLayer* metalLayer);
        
        virtual TextureId begin() final;
        virtual void present(TextureId surface) final;
        virtual PixelFormat pixelFormat() const final;
        virtual uint32_t width() const final;
        virtual uint32_t height() const final;
        
    };
}

#endif /* MetalSwapchain_hpp */
