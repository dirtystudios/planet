#include "DX12Backend.h"

#include "DX12Device.h"
#include "RenderBackend.h"
#include "DX12Swapchain.h"
#include "DX12Assert.h"

#include "DX12EnumAdapter.h"

namespace gfx {
    using namespace Microsoft::WRL;

    DX12Backend::DX12Backend(bool usePrebuiltShaders) {
        UINT factoryFlags = kDebugDx12 ? DXGI_CREATE_FACTORY_DEBUG : 0;
        DX12_CHECK(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&_factory)));

        /* warp adapter
        ComPtr<IDXGIAdapter> warpAdapter;
        DX12_CHECK(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
        */

        ComPtr<IDXGIAdapter1> hardwareAdapter;

        getHardwareAdapter(_factory.Get(), &hardwareAdapter);
        dg_assert(hardwareAdapter.Get() != nullptr, "DX12: No dx12 supported device found!");

        ComPtr<IDXGIAdapter3> castedAdapter;
        hardwareAdapter.As(&castedAdapter);

        _device.reset(new DX12Device(castedAdapter.Get(), &resourceManager, usePrebuiltShaders));
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
            ComPtr<IDXGIAdapter3> castedAdapter;
            DX12_CHECK(adapter.As(&castedAdapter));

            DX12_CHECK(castedAdapter->GetDesc(&adapterDesc));
            char buffer[128];
            wcstombs_s(0, buffer, 128, adapterDesc.Description, 128);

            LOG_D("DisplayAdapterDesc %d: %s", i, buffer);
            LOG_D("VendorID:DeviceID 0x%x:0x%x", adapterDesc.VendorId, adapterDesc.DeviceId);

            DXGI_QUERY_VIDEO_MEMORY_INFO memoryInfo = {};
            DX12_CHECK(castedAdapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &memoryInfo));
            LOG_D("Memory Budget: %llu MB | Using: %llu MB", memoryInfo.Budget >> 20, memoryInfo.CurrentUsage >> 20);

            ++i;
        }
    }

    void DX12Backend::getHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter) {
        ComPtr<IDXGIAdapter1> adapter;
        *ppAdapter = nullptr;

        for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select the Basic Render Driver adapter
                continue;
            }

            // Check to see if the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }

        *ppAdapter = adapter.Detach();
    }
}