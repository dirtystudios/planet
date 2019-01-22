#pragma once
#include "Resource.h"
#include "VertexLayoutDesc.h"
#include "ShaderType.h"
#include "PixelFormat.h"
#include "RenderPassInfo.h"

#include <wrl.h>
#include <d3d11.h>

namespace gfx {

    // these are used internally by renderdevice
    struct BlendStateDX11 final {
        Microsoft::WRL::ComPtr<ID3D11BlendState> bs;
    };

    struct RasterStateDX11 final {
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> rs;
    };

    struct DepthStateDX11 final {
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> ds;
    };

    // these are managed by resource manager

    struct PipelineStateDX11 : Resource {
        size_t vertexLayoutHandle{ 0 };
        size_t vertexShaderHandle{ 0 };

        size_t renderPassHandle{ 0 };

        ID3D11VertexShader* vertexShader{ 0 };
        ID3D11PixelShader* pixelShader{ 0 };
        ID3D11InputLayout* vertexLayout{ 0 };
        uint32_t vertexLayoutStride;
        D3D11_PRIMITIVE_TOPOLOGY topology;
        ID3D11BlendState* blendState;
        ID3D11RasterizerState* rasterState;
        ID3D11DepthStencilState* depthState;
    };

    struct InputLayoutDX11 : public Resource {
        Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
        VertexLayoutDesc layoutDesc;
        uint32_t stride;
    };

    struct BufferDX11 : public Resource {
        Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
    };

    struct TextureDX11 : public Resource {
        Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
        Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView> dsv;
        DXGI_FORMAT format;
        PixelFormat requestedFormat;
        uint32_t width;
        uint32_t height;
    };

    struct ShaderDX11 : public Resource {
        Microsoft::WRL::ComPtr<ID3DBlob> blob;
        ShaderType shaderType;
        ID3D11VertexShader *vertexShader;
        ID3D11PixelShader *pixelShader;
        ID3D11ComputeShader *computeShader;
    };

    struct RenderPassDX11 : public Resource {
        RenderPassInfo info;
    };
}