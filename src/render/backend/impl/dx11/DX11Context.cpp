#include "DX11Context.h"
#include "Hash.h"

namespace std {
    template<> struct hash<D3D11_VIEWPORT> {
        typedef D3D11_VIEWPORT argument_type;
        typedef std::size_t result_type;
        result_type operator()(argument_type const& s) const noexcept {
            result_type h = 0;
            HashCombine(h, s.TopLeftX);
            HashCombine(h, s.TopLeftY);
            HashCombine(h, s.Width);
            HashCombine(h, s.Height);
            HashCombine(h, s.MinDepth);
            HashCombine(h, s.MaxDepth);
            return h;
        }
    };
}

namespace gfx {

    void DX11Context::ClearRenderTargetView(ID3D11RenderTargetView* rtv, float r, float g, float b, float a) {
        float RGBA[4] = { r, g, b, a };
        m_devcon->ClearRenderTargetView(rtv, RGBA);
    }

    void DX11Context::ClearDepthStencil(ID3D11DepthStencilView* dsv, bool clearDepth, float depthVal, bool clearStencil, uint8_t stencilVal) {
        if (!clearDepth && !clearStencil)
            return;
        UINT cf = clearDepth ? (clearStencil ? (D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL) : D3D11_CLEAR_DEPTH) : D3D11_CLEAR_STENCIL;
        m_devcon->ClearDepthStencilView(dsv, cf, depthVal, stencilVal);
    }

    void DX11Context::SetRasterState(ID3D11RasterizerState* state) {
        m_pendingState.rasterState = state;
    }

    void DX11Context::SetDepthState(ID3D11DepthStencilState* state) {
        m_pendingState.depthState = state;
    }

    void DX11Context::SetBlendState(ID3D11BlendState* state) {
        m_pendingState.blendState = state;
    }

    void DX11Context::SetRenderTargets(std::array<ID3D11RenderTargetView*, 8> rtvs, uint8_t rtvCount, ID3D11DepthStencilView* depthStencil) {
        m_pendingState.rtvs = rtvs;
        m_pendingState.rtv_count = rtvCount;
        m_pendingState.dsv = depthStencil;
    }

    void DX11Context::SetViewport(const D3D11_VIEWPORT& vp) {
        m_pendingState.vphash = std::hash<D3D11_VIEWPORT>()(vp);
        m_pendingState.vp = vp;
    }

    void* DX11Context::MapBufferPointer(ID3D11Buffer* buffer, D3D11_MAP usage) {
        D3D11_BUFFER_DESC bufferDesc;
        buffer->GetDesc(&bufferDesc);
        if (bufferDesc.Usage != D3D11_USAGE_DYNAMIC) {
            LOG_E("DX11RenderDev: Only dynamic buffer type updating allowed.");
            return 0;
        }
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        DX11_CHECK(m_devcon->Map(buffer, 0, usage, 0, &mappedResource));
        return mappedResource.pData;
    }

    void DX11Context::UnMapBufferPointer(ID3D11Buffer* buffer) {
        m_devcon->Unmap(buffer, 0);
    }

    void DX11Context::UpdateBufferData(ID3D11Buffer* buffer, void* data, size_t len){
        D3D11_BUFFER_DESC bufferDesc;
        buffer->GetDesc(&bufferDesc);
        if (bufferDesc.Usage != D3D11_USAGE_DYNAMIC) {
            LOG_E("DX11RenderDev: Only dynamic buffer type updating allowed.");
            return;
        }
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        DX11_CHECK_RET(m_devcon->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
        memcpy(mappedResource.pData, data, len);

        m_devcon->Unmap(buffer, 0);
    }

    void DX11Context::UpdateSubResource(ID3D11Resource* tex, uint32_t subresource, const D3D11_BOX& box, const void* data, uint32_t rowPitch, uint32_t rowDepth) {
        m_devcon->UpdateSubresource(tex, subresource, &box, data, rowPitch, rowDepth);
    }

    void DX11Context::SetVertexCBuffer(uint32_t slot, ID3D11Buffer* buffer) {
        auto it = m_currentState.vsCBuffers.find(slot);
        if (it == m_currentState.vsCBuffers.end()) {
            m_pendingState.vsCBuffers.emplace(slot, buffer);
            m_pendingState.vsCBufferDirtySlots.emplace(slot);
        }
        else if (it->second != buffer) {
            m_pendingState.vsCBuffers[slot] = buffer;
            m_pendingState.vsCBufferDirtySlots.emplace(slot);
        }
    }

    void DX11Context::SetPixelCBuffer(uint32_t slot, ID3D11Buffer* buffer) {
        auto it = m_currentState.psCBuffers.find(slot);
        if (it == m_currentState.psCBuffers.end()) {
            m_pendingState.psCBuffers.emplace(slot, buffer);
            m_pendingState.psCBufferDirtySlots.emplace(slot);
        }
        else if (it->second != buffer) {
            m_pendingState.psCBuffers[slot] = buffer;
            m_pendingState.psCBufferDirtySlots.emplace(slot);
        }
    }

    void DX11Context::SetComputeCBuffer(uint32_t slot, ID3D11Buffer* buffer) {
        auto it = m_currentState.csCBuffers.find(slot);
        if (it == m_currentState.csCBuffers.end()) {
            m_pendingState.csCBuffers.emplace(slot, buffer);
            m_pendingState.csCBufferDirtySlots.emplace(slot);
        }
        else if (it->second != buffer) {
            m_pendingState.csCBuffers[slot] = buffer;
            m_pendingState.csCBufferDirtySlots.emplace(slot);
        }
    }

    void DX11Context::SetInputLayout(uint32_t stride, ID3D11InputLayout* layout) {
        m_pendingState.inputLayout = layout;
        m_pendingState.inputLayoutStride = stride;
    }

    void DX11Context::SetVertexBuffer(ID3D11Buffer* buffer) {
        m_pendingState.vertexBuffer = buffer;
    }

    void DX11Context::SetIndexBuffer(ID3D11Buffer* buffer) {
        m_pendingState.indexBuffer = buffer;
    }

    void DX11Context::SetVertexShader(ID3D11VertexShader* shader) {
        m_pendingState.vertexShader = shader;
    }

    void DX11Context::SetPixelShader(ID3D11PixelShader* shader) {
        m_pendingState.pixelShader = shader;
    }

    void DX11Context::SetComputeShader(ID3D11ComputeShader* shader) {
        m_pendingState.computeShader = shader;
    }

    void DX11Context::SetPixelShaderTexture(uint32_t slot, ID3D11ShaderResourceView* srv, ID3D11SamplerState* sampler) {
        auto it = m_currentState.psTextures.find(slot);
        auto its = m_currentState.psSamplers.find(slot);
        if (it == m_currentState.psTextures.end() || its == m_currentState.psSamplers.end()) {
            m_pendingState.psTextures.emplace(slot, srv);
            m_pendingState.psSamplers.emplace(slot, sampler);
            m_pendingState.psDirtyTextureSlots.emplace(slot);
        }
        else if (it->second != srv || its->second != sampler) {
            m_pendingState.psTextures[slot] = srv;
            m_pendingState.psSamplers[slot] = sampler;
            m_pendingState.psDirtyTextureSlots.emplace(slot);
        }
    }

    void DX11Context::SetVertexShaderTexture(uint32_t slot, ID3D11ShaderResourceView* srv, ID3D11SamplerState* sampler) {
        auto it = m_currentState.vsTextures.find(slot);
        auto its = m_currentState.vsSamplers.find(slot);
        if (it == m_currentState.vsTextures.end() || its == m_currentState.vsSamplers.end()) {
            m_pendingState.vsTextures.emplace(slot, srv);
            m_pendingState.vsSamplers.emplace(slot, sampler);
            m_pendingState.vsDirtyTextureSlots.emplace(slot);
        }
        else if (it->second != srv || its->second != sampler) {
            m_pendingState.vsTextures[slot] = srv;
            m_pendingState.vsSamplers[slot] = sampler;
            m_pendingState.vsDirtyTextureSlots.emplace(slot);
        }
    }

    void DX11Context::SetComputeShaderTexture(uint32_t slot, ID3D11ShaderResourceView* srv, ID3D11SamplerState* sampler) {
        auto it = m_currentState.csTextures.find(slot);
        auto its = m_currentState.csSamplers.find(slot);
        if (it == m_currentState.csTextures.end() || its == m_currentState.csSamplers.end()) {
            m_pendingState.csTextures.emplace(slot, srv);
            m_pendingState.csSamplers.emplace(slot, sampler);
            m_pendingState.csDirtyTextureSlots.emplace(slot);
        }
        else if (it->second != srv || its->second != sampler) {
            m_pendingState.csTextures[slot] = srv;
            m_pendingState.csSamplers[slot] = sampler;
            m_pendingState.csDirtyTextureSlots.emplace(slot);
        }
    }

    void DX11Context::SetComputeShaderUAV(uint32_t slot, ID3D11UnorderedAccessView* uav) {
        auto it = m_currentState.csUavs.find(slot);
        if (it == m_currentState.csUavs.end()) {
            m_pendingState.csUavs.emplace(slot, uav);
            m_pendingState.csDirtyUavSlots.emplace(slot);
        }
        else if (it->second != uav) {
            m_pendingState.csUavs[slot] = uav;
            m_pendingState.csDirtyUavSlots.emplace(slot);
        }
    }

    void DX11Context::SetPrimitiveType(D3D11_PRIMITIVE_TOPOLOGY top) {
        m_pendingState.primitiveType = top;
    }

    void DX11Context::DrawPrimitive(uint32_t startVertex, uint32_t numVertices, bool indexed, uint32_t baseVertexLocation) {

        if (m_pendingState.vertexShader != nullptr && m_currentState.vertexShader != m_pendingState.vertexShader)
            m_devcon->VSSetShader(m_pendingState.vertexShader, 0, 0);

        for (uint32_t slot : m_pendingState.vsCBufferDirtySlots)
            m_devcon->VSSetConstantBuffers(slot, static_cast<UINT>(1), &m_pendingState.vsCBuffers[slot]);

        // todo: format?
        if (m_pendingState.indexBuffer != nullptr && m_currentState.indexBuffer != m_pendingState.indexBuffer)
            m_devcon->IASetIndexBuffer(m_pendingState.indexBuffer, DXGI_FORMAT_R32_UINT, 0);

        if (m_pendingState.primitiveType != m_currentState.primitiveType)
            m_devcon->IASetPrimitiveTopology(m_pendingState.primitiveType);

        if (m_pendingState.inputLayout != nullptr && m_currentState.inputLayout != m_pendingState.inputLayout)
            m_devcon->IASetInputLayout(m_pendingState.inputLayout);

        if (m_pendingState.vertexBuffer != nullptr && m_currentState.vertexBuffer != m_pendingState.vertexBuffer) {
            uint32_t offset = 0;
            uint32_t stride = static_cast<UINT>(m_pendingState.inputLayoutStride);
            m_devcon->IASetVertexBuffers(0, 1u, &m_pendingState.vertexBuffer, &stride, &offset);
        }

        for (uint32_t slot : m_pendingState.vsDirtyTextureSlots) {
            // todo: batch these calls
            m_devcon->VSSetShaderResources(slot, 1u, &m_pendingState.vsTextures[slot]);
            m_devcon->VSSetSamplers(slot, 1u, &m_pendingState.vsSamplers[slot]);
        }

        for (uint32_t slot : m_pendingState.psDirtyTextureSlots) {
            m_devcon->PSSetShaderResources(slot, 1u, &m_pendingState.psTextures[slot]);
            m_devcon->PSSetSamplers(slot, 1u, &m_pendingState.psSamplers[slot]);
        }

        if (m_pendingState.pixelShader != nullptr && m_currentState.pixelShader != m_pendingState.pixelShader)
            m_devcon->PSSetShader(m_pendingState.pixelShader, 0, 0);

        for (uint32_t slot : m_pendingState.psCBufferDirtySlots)
            m_devcon->PSSetConstantBuffers(slot, 1u, &m_pendingState.psCBuffers[slot]);

        if (m_pendingState.rasterState != nullptr && m_currentState.rasterState != m_pendingState.rasterState)
            m_devcon->RSSetState(m_pendingState.rasterState);

        if (m_pendingState.vphash != m_currentState.vphash)
            m_devcon->RSSetViewports(1, &m_pendingState.vp);

        if (m_pendingState.blendState != nullptr && m_currentState.blendState != m_pendingState.blendState) {
            float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
            UINT sampleMask = 0xffffffff;
            m_devcon->OMSetBlendState(m_pendingState.blendState, blendFactor, sampleMask);
        }

        if (m_pendingState.depthState != nullptr && m_currentState.depthState != m_pendingState.depthState)
            m_devcon->OMSetDepthStencilState(m_pendingState.depthState, 1);

        if (m_pendingState.rtv_count != m_currentState.rtv_count || m_pendingState.dsv != m_currentState.dsv)
            m_devcon->OMSetRenderTargets(m_pendingState.rtv_count, m_pendingState.rtvs.data(), m_pendingState.dsv);

        // We use SWAP_FLIP_SEQUENTIAL with 11.3, which apparently needs this call everytime....ugh
#ifdef DX11_3_API
        m_devcon->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
#endif
        if (indexed)
            m_devcon->DrawIndexed(numVertices, startVertex, baseVertexLocation);
        else
            m_devcon->Draw(numVertices, startVertex);

        //cleanup state before set
        m_pendingState.psCBufferDirtySlots.clear();
        m_pendingState.psDirtyTextureSlots.clear();
        m_pendingState.vsCBufferDirtySlots.clear();
        m_pendingState.vsDirtyTextureSlots.clear();
        m_currentState = m_pendingState;
    }

    void DX11Context::Dispatch(uint32_t x, uint32_t y, uint32_t z) {
        if (m_pendingState.computeShader != nullptr && m_currentState.computeShader != m_pendingState.computeShader)
            m_devcon->CSSetShader(m_pendingState.computeShader, 0, 0);

        for (uint32_t slot : m_pendingState.csDirtyTextureSlots) {
            // todo: batch these calls
            m_devcon->CSSetShaderResources(slot, 1u, &m_pendingState.csTextures[slot]);
            if (auto sampler = m_pendingState.csSamplers[slot])
                m_devcon->CSSetSamplers(slot, 1u, &sampler);
        }

        for (uint32_t slot : m_pendingState.csDirtyUavSlots)
            m_devcon->CSSetUnorderedAccessViews(slot, 1u, &m_pendingState.csUavs[slot], nullptr);

        for (uint32_t slot : m_pendingState.csCBufferDirtySlots)
            m_devcon->CSSetConstantBuffers(slot, 1u, &m_pendingState.csCBuffers[slot]);

        m_devcon->Dispatch(x, y, z);

        //cleanup state before set
        m_pendingState.csCBufferDirtySlots.clear();
        m_pendingState.csDirtyTextureSlots.clear();
        m_pendingState.csDirtyUavSlots.clear();
        m_currentState = m_pendingState;
    }

    void DX11Context::ExecuteCommandList(ID3D11CommandList *pCommandList, BOOL RestoreContextState) {
        m_devcon->ExecuteCommandList(pCommandList, RestoreContextState);
    }
}