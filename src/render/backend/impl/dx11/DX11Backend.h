#pragma once
#include "DX11Device.h"
#include "DX11Swapchain.h"
#include "RenderBackend.h"
#include "ResourceManager.h"

#ifdef DX11_3_API
#include <d3d11_3.h>
#else
#include <d3d11.h>
#include <dxgi1_2.h>
#endif

#include <wrl.h>

namespace gfx {

class DX11Backend final : public RenderBackend {
private:
    const uint32_t kNumFrames = 2;

#ifdef DX11_3_API
    Microsoft::WRL::ComPtr<IDXGIFactory3> m_factory;
#else
    Microsoft::WRL::ComPtr<IDXGIFactory2> m_factory;
#endif
    ResourceManager resourceManager;
    std::unique_ptr<DX11Device> _device;
    std::vector<std::unique_ptr<DX11Swapchain>> _swapchains;

public:
    DX11Backend(bool usePrebuiltShaders);

    RenderDevice* getRenderDevice();
    void printDeviceInfo();
    Swapchain* createSwapchainForWindow(const SwapchainDesc& swapchainDesc, RenderDevice* device, void* windowHandle);
};
} // namespace gfx