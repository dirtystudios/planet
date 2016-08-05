#pragma once
#include "DX11ContextState.h"

#include "DX11Utils.h"

#include <unordered_map>
#include <wrl.h>

// Check if compiling as uwp windows app and use newer api instead 
#ifdef WINAPI_PARTITION_APP
#if WINAPI_FAMILY == WINAPI_FAMILY_PC_APP
#define DX11_3_API
#endif
#endif

#ifdef DX11_3_API
#include <d3d11_3.h>
#else
#include <d3d11.h>
#endif


namespace gfx {
    using namespace Microsoft::WRL;
    class DX11Context {
    private:
#ifdef DX11_3_API
        ComPtr<ID3D11DeviceContext3> m_devcon;
#else
        ComPtr<ID3D11DeviceContext> m_devcon;
#endif
        ID3D11RenderTargetView* m_renderTargetView;
        ID3D11DepthStencilView* m_depthStencilView;

        ID3D11SamplerState* m_defaultSampler;

        DX11ContextState m_pendingState, m_currentState;
    public:
        DX11Context(const ComPtr<ID3D11DeviceContext>& deviceContext) { DX11_CHECK(deviceContext.As(&m_devcon)); };
        ~DX11Context();
        void Clear(float r, float g, float b, float a);

        void* MapBufferPointer(ID3D11Buffer* buffer, D3D11_MAP usage);
        void UnMapBufferPointer(ID3D11Buffer* buffer);
        void UpdateBufferData(ID3D11Buffer* buffer, void* data, size_t len);
        void SetVertexCBuffer(size_t handle, uint32_t slot, ID3D11Buffer* buffer);
        void SetPixelCBuffer(size_t handle, uint32_t slot, ID3D11Buffer* buffer);

        void SetVertexBuffer(size_t handle, ID3D11Buffer* buffer);

        void SetIndexBuffer(size_t handle, ID3D11Buffer* buffer);

        void SetInputLayout(size_t handle, uint32_t stride, ID3D11InputLayout* layout);

        void SetRasterState(size_t handle, ID3D11RasterizerState* state);
        void SetDepthState(size_t handle, ID3D11DepthStencilState* state);
        void SetBlendState(size_t handle, ID3D11BlendState* state);

        void SetVertexShaderTexture(uint32_t slot, ID3D11ShaderResourceView* srv, ID3D11SamplerState* sampler);
        void SetPixelShaderTexture(uint32_t slot, ID3D11ShaderResourceView* srv, ID3D11SamplerState* sampler);

        void SetVertexShader(size_t handle, ID3D11VertexShader* shader);
        void SetPixelShader(size_t handle, ID3D11PixelShader* shader);

        void SetRenderTarget(ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* depthStencil);
        void SetViewport(uint32_t target, const D3D11_VIEWPORT& vp);

        void DrawPrimitive(D3D11_PRIMITIVE_TOPOLOGY primitiveType, uint32_t startVertex, uint32_t numVertices, bool indexed);
    };
}
