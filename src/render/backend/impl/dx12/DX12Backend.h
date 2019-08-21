#pragma once

#include "RenderBackend.h"
#include "DX12Device.h"
#include "DX12Swapchain.h"

#include <dxgi1_4.h>
#include <d3d12.h>
#include <wrl.h>

namespace gfx {

    class DX12Backend final : public RenderBackend {
    private:
        const uint32_t kNumFrames = 2;

        ComPtr<IDXGIFactory4> _factory;
        ResourceManager resourceManager;
        std::unique_ptr<DX12Device> _device;
        std::vector<std::unique_ptr<DX12Swapchain>> _swapchains;

    public:
        DX12Backend(bool usePrebuiltShaders);

        RenderDevice* getRenderDevice();
        void printDeviceInfo();
        Swapchain* createSwapchainForWindow(const SwapchainDesc& swapchainDesc, RenderDevice* device, void* windowHandle);
    };
} // namespace gfx