#include "DX12Swapchain.h"

#include "DGAssert.h"
#include "DX12Assert.h"
#include "DX12Device.h"
#include "DX12EnumAdapter.h"
#include <wrl.h>

namespace gfx {
    using namespace Microsoft::WRL;
    DX12Swapchain::DX12Swapchain(const SwapchainDesc& desc, ID3D12CommandQueue* cq, DX12Device* dev, ResourceManager* rm, ComPtr<IDXGISwapChain3> sc)
        : Swapchain(desc), _dev(dev), _rm(rm), _cq(cq) {
        dg_assert_nm(sc != nullptr);
        _sc.Swap(sc);
        DXGI_SWAP_CHAIN_DESC1 dx12Desc;
        DX12_CHECK(_sc->GetDesc1(&dx12Desc));
        _backbufferCount = dx12Desc.BufferCount;
        _currentBackBufferIndex = _sc->GetCurrentBackBufferIndex();

        _format = dx12Desc.Format;
        _reqFormat = desc.format;

        _srvHeap = std::make_unique<DX12CpuDescHeap>(_dev, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, "swapchainSrvHeap");
        _rtvHeap = std::make_unique<DX12CpuDescHeap>(_dev, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, "swapchainRtvHeap");

        DX12_CHECK(_dev->GetID3D12Dev()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_directAllocator)));

        ComPtr<ID3D12GraphicsCommandList> cmdList;
        DX12_CHECK(_dev->GetID3D12Dev()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _directAllocator.Get(), nullptr, IID_PPV_ARGS(&_cmdList)));

        onSwapchainResize(desc.width, desc.height);
    }

    TextureId DX12Swapchain::begin() {
        //jake: todo: lock/wait?
        dg_assert_nm(_currentBackBufferIndex > _backBuffers.size());
        return _backBuffers[_currentBackBufferIndex];
    }

    void DX12Swapchain::onSwapchainResize(uint32_t width, uint32_t height) {
        if (_backBuffers.size() != 0)
            dg_assert_fail("todo: jake: implement resize");
        // need wait for gpu?

        //DX12_CHECK_RET(_sc->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0));
        // create textures, desc
        for (UINT i = 0; i < _backbufferCount; ++i) {
            ComPtr<ID3D12Resource> buffer;
            DX12_CHECK(_sc->GetBuffer(i, IID_PPV_ARGS(&buffer)));

            D3D12_CPU_DESCRIPTOR_HANDLE rtvDescCpuHandle = _rtvHeap->AllocateDescriptor();

            rtvDescCpuHandle = _rtvHeap->AllocateDescriptor();
            _dev->GetID3D12Dev()->CreateRenderTargetView(buffer.Get(), nullptr, rtvDescCpuHandle);

            auto srvDescCpuHandle = _srvHeap->AllocateDescriptor();

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Format = _format;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;
            _dev->GetID3D12Dev()->CreateShaderResourceView(buffer.Get(), &srvDesc, srvDescCpuHandle);

            auto usageflags = TextureUsageFlags::ShaderRead | TextureUsageFlags::RenderTarget;;

            TextureDX12* tmp = new TextureDX12();
            tmp->currentState = D3D12_RESOURCE_STATE_PRESENT; // this is a guess
            tmp->dsv = { 0 };
            tmp->format = _format;
            tmp->requestedFormat = _reqFormat;
            tmp->height = height;
            tmp->width = width;
            tmp->rtv = rtvDescCpuHandle;
            tmp->usage = usageflags.underlying_value;
            tmp->srv = srvDescCpuHandle;

            _backBuffers.push_back(_rm->AddResource(tmp));
        }
    }

    void DX12Swapchain::present(TextureId surface) {
        //jake: todo: lock/wait?
        dg_assert_nm(_backBuffers[_currentBackBufferIndex] == surface);
        auto tex = _rm->GetResource<TextureDX12>(surface);
        dg_assert_nm(tex);

        if (tex->currentState != D3D12_RESOURCE_STATE_PRESENT) {
            _cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(tex->resource.Get(), tex->currentState, D3D12_RESOURCE_STATE_PRESENT));
            DX12_CHECK(_cmdList->Close());
            DX12_CHECK(_cmdList->Reset(_directAllocator.Get(), nullptr));
            ID3D12CommandList* ppCommandLists[] = { _cmdList.Get() };
            _cq->ExecuteCommandLists(1, ppCommandLists);
        }

        DX12_CHECK(_sc->Present(0, 0));

        _currentBackBufferIndex = _sc->GetCurrentBackBufferIndex();
    }
}