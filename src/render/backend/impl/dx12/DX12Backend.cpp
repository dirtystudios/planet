#include "DX12Backend.h"

#include "DX12Device.h"
#include "RenderBackend.h"
#include "DX12Swapchain.h"
#include "DX12Assert.h"

#include "DX12EnumAdapter.h"

namespace gfx {
    using namespace Microsoft::WRL;

    DX12Backend::DX12Backend(bool usePrebuiltShaders) {
        DX12_CHECK(CreateDXGIFactory1(IID_PPV_ARGS(&_factory)));
        _device.reset(new DX12Device(&resourceManager, usePrebuiltShaders));
        dg_assert_nm(_device.get() != nullptr);
    }

    Swapchain* DX12Backend::createSwapchainForWindow(const SwapchainDesc& swapchainDesc, RenderDevice* device, void* windowHandle) {

        // Describe and create the swap chain.
        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
        swapChainDesc.BufferCount = kNumFrames;
        swapChainDesc.Width = swapchainDesc.width;
        swapChainDesc.Height = swapchainDesc.height;
        swapChainDesc.Format = SafeGet(PixelFormatDX12, swapchainDesc.format);;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.SampleDesc.Count = 1;

        ComPtr<IDXGISwapChain1> swapChain1;
        DX12_CHECK_RET0(_factory->CreateSwapChainForHwnd(
            _device->GetCommandQueue(),		// Swap chain needs the queue so that it can force a flush on it.
            static_cast<HWND>(windowHandle),
            &swapChainDesc,
            nullptr,
            nullptr,
            &swapChain1
        ));

        // disable fullscreen transitions.
        DX12_CHECK_RET0(_factory->MakeWindowAssociation(static_cast<HWND>(windowHandle), DXGI_MWA_NO_ALT_ENTER));

        ComPtr<IDXGISwapChain3> sc3;
        DX12_CHECK_RET0(swapChain1.As(&sc3));
        D3D_SET_OBJECT_NAME_A(sc3, std::string("Swapchain").c_str());

        _swapchains.emplace_back(new DX12Swapchain(swapchainDesc, _device->GetCommandQueue(), dynamic_cast<DX12Device*>(device), &resourceManager, sc3));
        return dynamic_cast<Swapchain*>(_swapchains.back().get());
    }

    RenderDevice* DX12Backend::getRenderDevice() {
        return _device.get();
    }

    void DX12Backend::printDeviceInfo() {
        ComPtr<IDXGIAdapter> adapter;
        UINT i = 0;
        while (_factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND) {
            auto adapterDesc = DXGI_ADAPTER_DESC();

            DX12_CHECK(adapter->GetDesc(&adapterDesc));
            char buffer[128];
            wcstombs_s(0, buffer, 128, adapterDesc.Description, 128);

            LOG_D("DisplayAdapterDesc %d: %s", i, buffer);
            LOG_D("VendorID:DeviceID 0x%x:0x%x", adapterDesc.VendorId, adapterDesc.DeviceId);
            ++i;
        }
    }
}