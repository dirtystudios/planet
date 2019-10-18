#pragma once

#include "Swapchain.h"
#include "ResourceTypes.h"
#include "DX12CpuDescHeap.h"

#include <vector>
#include <memory>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl.h>

namespace gfx {
    class DX12Device;
    class ResourceManager;

    class DX12Swapchain final : public Swapchain {
    private:
        Microsoft::WRL::ComPtr<IDXGISwapChain3> _sc;
        std::vector<TextureId> _backBuffers; // maps 'swapchain index' to textureid
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator> _directAllocator{ nullptr };
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _cmdList;
        ID3D12CommandQueue* _cq;
        UINT _backbufferCount{ 0 };
        UINT _currentBackBufferIndex{ 0 };
        DXGI_FORMAT _format;
        PixelFormat _reqFormat;
        DX12Device* _dev{ nullptr };
        ResourceManager* _rm{ nullptr };

        std::unique_ptr<DX12CpuDescHeap> _srvHeap{ nullptr };
        std::unique_ptr<DX12CpuDescHeap> _rtvHeap{ nullptr };

    public:
        DX12Swapchain(const SwapchainDesc& desc, ID3D12CommandQueue* cq, DX12Device* dev, ResourceManager* rm, Microsoft::WRL::ComPtr<IDXGISwapChain3> sc);
        TextureId begin() final;
        void present(TextureId surface) final;
        void onSwapchainResize(uint32_t width, uint32_t height) final;
    };
} // namespace gfx