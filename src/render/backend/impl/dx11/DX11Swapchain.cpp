#include "DX11Swapchain.h"
#include "DGAssert.h"
#include "DX11Debug.h"
#include "DX11Device.h"
#include "DX11EnumAdapter.h"
#include <wrl.h>

namespace gfx {
    using namespace Microsoft::WRL;
    DX11Swapchain::DX11Swapchain(const SwapchainDesc& desc, DX11Device* dev, ResourceManager* rm, ComPtr<IDXGISwapChain1> sc)
    : Swapchain(desc), _dev(dev), _rm(rm) {
        dg_assert_nm(sc != nullptr);
        _sc.Swap(sc);

        _format = SafeGet(PixelFormatDX11, desc.format);
        onSwapchainResize(desc.width, desc.height);
    }

    TextureId DX11Swapchain::begin() {
        return _backBufferId;
    }

    void DX11Swapchain::onSwapchainResize(uint32_t width, uint32_t height) {
        if (_backBufferId != 0)
            _rm->DestroyResource<TextureDX11>(_backBufferId);

        DX11_CHECK_RET(_sc->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0));

        // blt model can only address first idx backbuffer
        // higher numbers can be 'read' but w/e
        // texture ptr stays constant for current backbuffer as long as swapchain isnt recreated

        ComPtr<ID3D11Texture2D> backbuffer;
        DX11_CHECK_RET(_sc->GetBuffer(0, IID_PPV_ARGS(&backbuffer)));
        D3D_SET_OBJECT_NAME_A(backbuffer, std::string("BackbufferTex").c_str());

        ComPtr<ID3D11ShaderResourceView> srv;
        DX11_CHECK_RET(_dev->GetID3D11Dev()->CreateShaderResourceView(backbuffer.Get(), NULL, &srv));
        dg_assert(srv, "srv creation failed for backbuffer");
        D3D_SET_OBJECT_NAME_A(srv, std::string("BackbufferSRV").c_str());

        ComPtr<ID3D11RenderTargetView> rtv;
        DX11_CHECK_RET(_dev->GetID3D11Dev()->CreateRenderTargetView(backbuffer.Get(), NULL, &rtv));
        dg_assert(rtv, "rtv creation failed for backbuffer");
        D3D_SET_OBJECT_NAME_A(rtv, std::string("BackbufferRTV").c_str());

        TextureDX11 *backBufferTex = new TextureDX11();
        backBufferTex->texture.Swap(backbuffer);
        backBufferTex->srv.Swap(srv);
        backBufferTex->rtv.Swap(rtv);
        backBufferTex->format = _format;
        backBufferTex->requestedFormat = _reqFormat;
        backBufferTex->width = width;
        backBufferTex->height = height;

        _backBufferId = _rm->AddResource(backBufferTex);
    }

    void DX11Swapchain::present(TextureId surface) {
        dg_assert_nm(surface == _backBufferId);

        DX11_CHECK(_sc->Present(0, 0));
    }
}