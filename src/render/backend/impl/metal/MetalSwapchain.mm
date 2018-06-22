//
//  MetalSwapchain.cpp
//  planet
//
//  Created by Eugene Sturm on 4/12/18.
//

#import "MetalSwapchain.h"
#import "ResourceManager.h"
#import <algorithm>
#import "MetalEnumAdapter.h"
using namespace gfx;



MetalSwapchain::MetalSwapchain(id<MTLCommandQueue> commandQueue, ResourceManager* resourceManager, CAMetalLayer* metalLayer)
: _metalLayer(metalLayer)
, _commandQueue(commandQueue)
, _inflightSemaphore(dispatch_semaphore_create(kImageCount))
, _resourceManager(resourceManager)
{
    for (int idx = 0; idx < kImageCount; ++idx) {
        _swapchainImages[idx] = new MetalSwapchainImage(this);
        _resourceManager->AddResource(_swapchainImages[idx]);
    }
}

TextureId MetalSwapchain::begin()
{
    dispatch_semaphore_wait(_inflightSemaphore, DISPATCH_TIME_FOREVER);
    MetalSwapchainImage* swapchainImage = _swapchainImages[_imageIdx];
    @autoreleasepool {
        swapchainImage->drawable = [[_metalLayer nextDrawable] retain];
    }
    
    swapchainImage->mtlTexture = [swapchainImage->drawable texture];
    swapchainImage->externalFormat = MetalEnumAdapter::fromMTL(swapchainImage->mtlTexture.pixelFormat);
    
    _imageIdx = (_imageIdx + 1) % kImageCount;
    return swapchainImage->resourceId;
}

PixelFormat MetalSwapchain::pixelFormat() const
{
    return MetalEnumAdapter::fromMTL(_metalLayer.pixelFormat);
}

uint32_t MetalSwapchain::width() const
{
    return _metalLayer.drawableSize.width;
}

uint32_t MetalSwapchain::height() const
{
    return _metalLayer.drawableSize.height;
}

void MetalSwapchain::present(TextureId surface)
{
    auto it = std::find_if(_swapchainImages.begin(), _swapchainImages.end(), [&](MetalSwapchainImage* image) { return image->resourceId == surface; });
    if (it == end(_swapchainImages)) {
        return;
    }
    
    MetalSwapchainImage* swapchainImage = *it;
    
    id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
    [commandBuffer enqueue];
    
    [commandBuffer pushDebugGroup:@"Present"];
    
    __block dispatch_semaphore_t blockSemaphore = _inflightSemaphore;
    [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
        dispatch_semaphore_signal(blockSemaphore);
    }];
    
    [commandBuffer presentDrawable:swapchainImage->drawable];
    [commandBuffer popDebugGroup];
    [commandBuffer commit];
    
    [swapchainImage->drawable release];
    swapchainImage->drawable = nil;
}
