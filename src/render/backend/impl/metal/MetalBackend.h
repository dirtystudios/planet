//
//  MetalBackend.h
//  planet
//
//  Created by Eugene Sturm on 4/12/18.
//

#ifndef MetalBackend_h
#define MetalBackend_h

#import "RenderBackend.h"
#include <memory>
#include <vector>

namespace gfx
{
    class MetalSwapchain;
    class MetalDevice;
    class ResourceManager;
    class RenderDevice;
    
    class MetalBackend : public RenderBackend
    {
    private:
        std::unique_ptr<MetalDevice> _device;
        std::unique_ptr<ResourceManager> _resourceManager;
        std::vector<MetalSwapchain*> _swapchains;
    public:
        MetalBackend();
        
        virtual RenderDevice* getRenderDevice() final;
        virtual Swapchain* createSwapchainForWindow(const SwapchainDesc& swapchainDesc, RenderDevice* device, void* windowHandle) final;
    };
}
#endif /* MetalBackend_hpp */
