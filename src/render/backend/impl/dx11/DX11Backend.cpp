#include "DX11Backend.h"

#include "DX11Device.h"
#include "RenderBackend.h"
#include "DX11Swapchain.h"

namespace gfx {
    using namespace Microsoft::WRL;

    DX11Backend::DX11Backend(bool usePrebuiltShaders) {
        DX11_CHECK(CreateDXGIFactory1(IID_PPV_ARGS(&m_factory)));
        _device.reset(new DX11Device(&resourceManager, usePrebuiltShaders));
        dg_assert_nm(_device.get() != nullptr);
    }

    Swapchain* DX11Backend::createSwapchainForWindow(const SwapchainDesc& swapchainDesc, RenderDevice* device, void* windowHandle) {
        DXGI_SWAP_CHAIN_DESC1 sd1;
        sd1.Width = swapchainDesc.width;
        sd1.Height = swapchainDesc.height;
        sd1.Format = SafeGet(PixelFormatDX11, swapchainDesc.format);
        sd1.Stereo = FALSE;
        sd1.SampleDesc.Count = 1; // not used in flip model
        sd1.SampleDesc.Quality = 0;
        sd1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
        sd1.BufferCount = 1;
        sd1.Scaling = DXGI_SCALING_STRETCH;
        sd1.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; // using discard as its supported on win7, todo: figure out how to get to use DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL with runtime check?
        sd1.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
        sd1.Flags = 0;

        ComPtr<IDXGISwapChain1> sc;
        DX11_CHECK_RET0(m_factory->CreateSwapChainForHwnd(_device->GetID3D11Dev(), static_cast<HWND>(windowHandle), &sd1, NULL, NULL, &sc));
        D3D_SET_OBJECT_NAME_A(sc, std::string("Swapchain").c_str());

        _swapchains.emplace_back(new DX11Swapchain(swapchainDesc, dynamic_cast<DX11Device*>(device), &resourceManager, sc));
        return dynamic_cast<Swapchain*>(_swapchains.back().get());
    }

    RenderDevice* DX11Backend::getRenderDevice() {
        return _device.get();
    }

    void DX11Backend::printDeviceInfo() {
        ComPtr<IDXGIAdapter> adapter;
        UINT i = 0;
        while (m_factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND) {
            auto adapterDesc = DXGI_ADAPTER_DESC();

            DX11_CHECK(adapter->GetDesc(&adapterDesc));
            char buffer[128];
            wcstombs_s(0, buffer, 128, adapterDesc.Description, 128);

            LOG_D("DisplayAdapterDesc %d: %s", i, buffer);
            LOG_D("VendorID:DeviceID 0x%x:0x%x", adapterDesc.VendorId, adapterDesc.DeviceId);
            ++i;
        }
    }
} 