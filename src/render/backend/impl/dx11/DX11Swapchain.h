#pragma once

#include "Swapchain.h"
#include "ResourceTypes.h"

#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl.h>

namespace gfx {
class DX11Device;
class ResourceManager;

class DX11Swapchain final: public Swapchain {
private:
    Microsoft::WRL::ComPtr<IDXGISwapChain1> _sc;
    TextureId _backBufferId{ 0 };
    DXGI_FORMAT _format;
    PixelFormat _reqFormat;
    DX11Device* _dev;
    ResourceManager* _rm;

public:
    DX11Swapchain(const SwapchainDesc& desc, DX11Device* dev, ResourceManager* rm, Microsoft::WRL::ComPtr<IDXGISwapChain1> sc);
    TextureId begin() final;
    void present(TextureId surface) final;
    void onSwapchainResize(uint32_t width, uint32_t height) final;
};
} // namespace gfx