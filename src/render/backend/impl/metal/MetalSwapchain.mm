//
//  MetalSwapchain.mm
//  planet
//
//  Created by Eugene Sturm on 4/12/18.
//

#import <algorithm>
#import "MetalSwapchain.h"
#import "ResourceManager.h"
#import "MetalEnumAdapter.h"
using namespace gfx;



MetalSwapchain::MetalSwapchain(const SwapchainDesc& desc, id<MTLCommandQueue> commandQueue, ResourceManager* resourceManager, MetalView* metalView)
: Swapchain(desc)
, _metalView(metalView)
, _metalLayer((CAMetalLayer*)metalView.layer)
, _commandQueue(commandQueue)
, _inflightSemaphore(dispatch_semaphore_create(kImageCount))
, _resourceManager(resourceManager)
{
    onSwapchainResize(width(), height());
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

void MetalSwapchain::onSwapchainResize(uint32_t width, uint32_t height)
{
    CGSize size = CGSizeMake(width, height);
    
    NSScreen* screen = _metalView.window.screen ?: [NSScreen mainScreen];
    size.width *= screen.backingScaleFactor;
    size.height *= screen.backingScaleFactor;
    
    _metalLayer.drawableSize = size;
}
