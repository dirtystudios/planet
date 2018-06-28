//
//  MetalBackend.cpp
//  planet
//
//  Created by Eugene Sturm on 4/12/18.
//

#import "MetalBackend.h"
#import "MetalSwapchain.h"
#import "MetalDevice.h"
#import "MetalView.h"
#import "MetalEnumAdapter.h"
#import "ResourceManager.h"



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
    
    MetalView* view = [[MetalView alloc] initWithFrame:window.contentView.frame];
    [window.contentView addSubview:view];    
    CAMetalLayer* metalLayer = (CAMetalLayer*)view.layer;
    metalLayer.device = metalDevice->getMTLDevice();
    metalLayer.pixelFormat = MetalEnumAdapter::toMTL(swapchainDesc.format);
    
    MetalSwapchain* swapchain = new MetalSwapchain(swapchainDesc, metalDevice->getMTLCommandQueue(), _resourceManager.get(), view);
    
    _swapchains.push_back(swapchain);
    
    return reinterpret_cast<Swapchain*>(swapchain);
}
