#include "DX12Swapchain.h"

#include "DGAssert.h"
#include "DX12Assert.h"
#include "DX12Device.h"
#include "DX12EnumAdapter.h"
#include <wrl.h>

namespace gfx {
    using namespace Microsoft::WRL;
    DX12Swapchain::DX12Swapchain(const SwapchainDesc& desc, ID3D12CommandQueue* cq, DX12Device* dev, ResourceManager* rm, ComPtr<IDXGISwapChain3> sc)
        : Swapchain(desc), _dev(dev), _rm(rm) {
        dg_assert_nm(sc != nullptr);
        _sc.Swap(sc);

        _format = SafeGet(PixelFormatDX12, desc.format);
        onSwapchainResize(desc.width, desc.height);
    }

    TextureId DX12Swapchain::begin() {
        return _backBufferId;
    }

    void DX12Swapchain::onSwapchainResize(uint32_t width, uint32_t height) {
        if (_backBufferId != 0)
            _rm->DestroyResource<TextureDX12>(_backBufferId);

        DX12_CHECK_RET(_sc->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0));

        //todo:
    }

    void DX12Swapchain::present(TextureId surface) {
        dg_assert_nm(surface == _backBufferId);

        DX12_CHECK(_sc->Present(0, 0));
    }
}