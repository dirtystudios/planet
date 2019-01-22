#pragma once
#include "DX11ContextState.h"

#include "DX11Debug.h"

#include <unordered_map>
#include <array>
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
    class DX11Context {
    private:
#ifdef DX11_3_API
        Microsoft::WRL::ComPtr<ID3D11DeviceContext3> m_devcon;
#else
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_devcon;
#endif
        DX11ContextState m_pendingState, m_currentState;
    public:

        // todo: switch this over to use comptr for resources, now that its deferred context, things might get ugly with deleting
        DX11Context(Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext) { m_devcon.Swap(deviceContext); };
        ID3D11DeviceContext* GetD3D11Context() { return m_devcon.Get(); };

        void* MapBufferPointer(ID3D11Buffer* buffer, D3D11_MAP usage);
        void UnMapBufferPointer(ID3D11Buffer* buffer);

        void UpdateBufferData(ID3D11Buffer* buffer, void* data, size_t len);
        void UpdateSubResource(ID3D11Resource* tex, uint32_t subresource, const D3D11_BOX& box, const void* data, uint32_t rowPitch, uint32_t rowDepth);

        void ClearRenderTargetView(ID3D11RenderTargetView* rtv, float r, float g, float b, float a);
        void ClearDepthStencil(ID3D11DepthStencilView* dsv, bool clearDepth, float depthVal, bool clearStencil, uint8_t stencilVal);

        void SetVertexCBuffer(uint32_t slot, ID3D11Buffer* buffer);
        void SetPixelCBuffer(uint32_t slot, ID3D11Buffer* buffer);

        void SetVertexBuffer(ID3D11Buffer* buffer);
        void SetIndexBuffer(ID3D11Buffer* buffer);

        void SetInputLayout(uint32_t stride, ID3D11InputLayout* layout);

        void SetRasterState(ID3D11RasterizerState* state);
        void SetDepthState(ID3D11DepthStencilState* state);
        void SetBlendState(ID3D11BlendState* state);

        void SetVertexShaderTexture(uint32_t slot, ID3D11ShaderResourceView* srv, ID3D11SamplerState* sampler);
        void SetPixelShaderTexture(uint32_t slot, ID3D11ShaderResourceView* srv, ID3D11SamplerState* sampler);

        void SetVertexShader(ID3D11VertexShader* shader);
        void SetPixelShader(ID3D11PixelShader* shader);
        void SetComputeShader(ID3D11ComputeShader* shader);

        void SetRenderTargets(std::array<ID3D11RenderTargetView*,8> rtvs, uint8_t rtvCount, ID3D11DepthStencilView* depthStencil);
        void SetViewport(const D3D11_VIEWPORT& vp);

        void SetPrimitiveType(D3D11_PRIMITIVE_TOPOLOGY top);

        void DrawPrimitive(uint32_t startVertex, uint32_t numVertices, bool indexed, uint32_t baseVertexLocation = 0);

        void ExecuteCommandList(ID3D11CommandList *pCommandList, BOOL RestoreContextState);
    };
}
