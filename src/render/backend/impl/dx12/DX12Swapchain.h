#pragma once

#include "Swapchain.h"
#include "ResourceTypes.h"

#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl.h>

namespace gfx {
    class DX12Device;
    class ResourceManager;

    class DX12Swapchain final : public Swapchain {
    private:
        Microsoft::WRL::ComPtr<IDXGISwapChain3> _sc;
        TextureId _backBufferId{ 0 };
        DXGI_FORMAT _format;
        PixelFormat _reqFormat;
        DX12Device* _dev;
        ResourceManager* _rm;

    public:
        DX12Swapchain(const SwapchainDesc& desc, ID3D12CommandQueue* cq, DX12Device* dev, ResourceManager* rm, Microsoft::WRL::ComPtr<IDXGISwapChain3> sc);
        TextureId begin() final;
        void present(TextureId surface) final;
        void onSwapchainResize(uint32_t width, uint32_t height) final;
    };
} // namespace gfx