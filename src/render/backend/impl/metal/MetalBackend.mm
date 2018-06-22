//
//  MetalBackend.cpp
//  planet
//
//  Created by Eugene Sturm on 4/12/18.
//

#import "MetalBackend.h"
#import "MetalSwapchain.h"
#import "MetalDevice.h"
#import <AppKit/AppKit.h>
#import "MetalEnumAdapter.h"
#import "ResourceManager.h"

@interface MetalView2 : NSView
@end

@implementation MetalView2

+ (Class)layerClass {
    return [CAMetalLayer class];
}

- (id)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    
    if (self) {
        self.wantsLayer = YES;
        self.layer = [CAMetalLayer layer];
    }
    
    return self;
}

@end

using namespace gfx;

MetalBackend::MetalBackend()
{
    _resourceManager.reset(new ResourceManager());
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    _device.reset(new MetalDevice(device, _resourceManager.get()));
    
}

RenderDevice* MetalBackend::getRenderDevice()
{
    return reinterpret_cast<RenderDevice*>(_device.get());
}

Swapchain* MetalBackend::createSwapchainForWindow(const SwapchainDesc& swapchainDesc, RenderDevice* device, void* windowHandle)
{
    NSWindow* window = reinterpret_cast<NSWindow*>(windowHandle);
    MetalDevice* metalDevice = reinterpret_cast<MetalDevice*>(device);
    
    MetalView2* view = [[MetalView2 alloc] initWithFrame:window.contentView.frame];
    [window.contentView addSubview:view];
    CAMetalLayer* metalLayer = (CAMetalLayer*)view.layer;
    metalLayer.device = metalDevice->getMTLDevice();
    metalLayer.pixelFormat = MetalEnumAdapter::toMTL(swapchainDesc.format);
    metalLayer.drawableSize = CGSizeMake(swapchainDesc.width, swapchainDesc.height);
    
    MetalSwapchain* swapchain = new MetalSwapchain(metalDevice->getMTLCommandQueue(), _resourceManager.get(), metalLayer);
    
    _swapchains.push_back(swapchain);
    
    return reinterpret_cast<Swapchain*>(swapchain);
}
