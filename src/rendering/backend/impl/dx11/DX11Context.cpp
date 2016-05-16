#include "DX11Context.h"

namespace graphics {

    void DX11Context::Clear(float r, float g, float b, float a) {
        float RGBA[4] = { r, g, b, a };
        m_devcon->ClearRenderTargetView(m_renderTargetView, RGBA);
        m_devcon->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
    }

    void DX11Context::SetRasterState(size_t handle, ID3D11RasterizerState* state) {
        if (m_currentState.rasterStateHash == handle) {
            m_pendingState.rasterStateHash = handle;
            m_pendingState.rasterState = m_currentState.rasterState;
            return;
        }
        m_pendingState.rasterStateHash = handle;
        m_pendingState.rasterState = state;
        return;
    }

    void DX11Context::SetDepthState(size_t handle, ID3D11DepthStencilState* state) {
        if (m_currentState.depthStateHash == handle) {
            m_pendingState.depthStateHash = handle;
            m_pendingState.depthState = m_currentState.depthState;
            return;
        }

        m_pendingState.depthStateHash = handle;
        m_pendingState.depthState = state;
        return;
    }

    void DX11Context::SetBlendState(size_t handle, ID3D11BlendState* state) {
        if (m_currentState.blendStateHash == handle) {
            m_pendingState.blendStateHash = handle;
            m_pendingState.blendState = m_currentState.blendState;
            return;
        }
        m_pendingState.blendStateHash = handle;
        m_pendingState.blendState = state;
        return;
    }

    void DX11Context::SetRenderTarget(ID3D11RenderTargetView* rtv, ID3D11DepthStencilView* depthStencil) {
        //m_currentState.depthState = depthStencil;
        m_renderTargetView = rtv;
        m_depthStencilView = depthStencil;

        // calling this here, not sure if i *should*
        m_devcon->OMSetRenderTargets(1, &rtv, depthStencil);
    }

    void DX11Context::SetViewport(uint32_t target, const D3D11_VIEWPORT& vp) {
        m_devcon->RSSetViewports(target, &vp);
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

    void DX11Context::SetVertexCBuffer(size_t handle, uint32_t slot, ID3D11Buffer* buffer) {
        if (m_currentState.vsCBuffer == buffer) {
            m_pendingState.vsCBuffer = buffer;
            return;
        }
        m_pendingState.vsCBuffer = buffer;
        /*if (std::find(m_pendingState.vsCBufferDirtySlots.begin(), m_pendingState.vsCBufferDirtySlots.end(), slot) == m_pendingState.vsCBufferDirtySlots.end()) {
            m_pendingState.vsCBufferDirtySlots.push_back(slot);
        }*/
    }

    void DX11Context::SetPixelCBuffer(size_t handle, uint32_t slot, ID3D11Buffer* buffer) {
        if (m_currentState.psCBuffer == buffer) {
            m_pendingState.psCBuffer = buffer;
            return;
        }
        m_pendingState.psCBuffer = buffer;
        /*if (std::find(m_pendingState.psCBufferDirtySlots.begin(), m_pendingState.psCBufferDirtySlots.end(), slot) == m_pendingState.psCBufferDirtySlots.end()) {
            m_pendingState.psCBufferDirtySlots.push_back(slot);
        }*/
    }

    void DX11Context::SetInputLayout(size_t handle, uint32_t stride, ID3D11InputLayout* layout) {
        if (m_currentState.inputLayoutHandle == handle) {
            m_pendingState.inputLayoutHandle = handle;
            m_pendingState.inputLayout = m_currentState.inputLayout;
            m_pendingState.inputLayoutStride = stride;
            return;
        }
        m_pendingState.inputLayoutHandle = handle;
        m_pendingState.inputLayout = layout;
        m_pendingState.inputLayoutStride = stride;
    }

    void DX11Context::SetVertexBuffer(size_t handle, ID3D11Buffer* buffer) {
        if (m_currentState.vertexBufferHandle == handle) {
            m_pendingState.vertexBufferHandle = m_currentState.vertexBufferHandle;
            m_pendingState.vertexBuffer = m_currentState.vertexBuffer;
            return;
        }

        m_pendingState.vertexBufferHandle = handle;
        m_pendingState.vertexBuffer = buffer;
    }

    void DX11Context::SetIndexBuffer(size_t handle, ID3D11Buffer* buffer) {
        if (m_currentState.indexBufferHandle == handle) {
            m_pendingState.indexBufferHandle = m_currentState.indexBufferHandle;
            m_pendingState.indexBuffer = m_currentState.indexBuffer;
            return;
        }

        m_pendingState.indexBufferHandle = handle;
        m_pendingState.indexBuffer = buffer;
    }

    void DX11Context::SetVertexShader(size_t handle, ID3D11VertexShader* shader) {
        if (m_currentState.vertexShaderHandle == handle) {
            m_pendingState.vertexShaderHandle = m_currentState.vertexShaderHandle;
            m_pendingState.vertexShader = m_currentState.vertexShader;
            return;
        }

        m_pendingState.vertexShaderHandle = handle;
        m_pendingState.vertexShader = shader;
    }

    void DX11Context::SetPixelShader(size_t handle, ID3D11PixelShader* shader) {
        if (m_currentState.pixelShaderHandle == handle) {
            m_pendingState.pixelShaderHandle = m_currentState.pixelShaderHandle;
            m_pendingState.pixelShader = m_currentState.pixelShader;
            return;
        }

        m_pendingState.pixelShaderHandle = handle;
        m_pendingState.pixelShader = shader;
    }

    void DX11Context::SetPixelShaderTexture(uint32_t slot, ID3D11ShaderResourceView* srv, ID3D11SamplerState* sampler) {
        m_defaultSampler = sampler;
        auto it = m_currentState.psTextures.find(slot);
        if (it == m_currentState.psTextures.end()) {
            m_pendingState.psTextures.emplace(slot, srv);
            m_pendingState.psDirtyTextureSlots.emplace_back((uint32_t)slot);
        }
        else if (it->second != srv) {
            m_pendingState.psTextures.at(slot) = srv;
            m_pendingState.psDirtyTextureSlots.emplace_back((uint32_t)slot);
        }
    }

    void DX11Context::SetVertexShaderTexture(uint32_t slot, ID3D11ShaderResourceView* srv, ID3D11SamplerState* sampler) {
        m_defaultSampler = sampler;
        auto it = m_currentState.vsTextures.find(slot);
        if (it == m_currentState.vsTextures.end()) {
            m_pendingState.vsTextures.emplace(slot, srv);
            m_pendingState.vsDirtyTextureSlots.emplace_back((uint32_t)slot);
        }
        else if (it->second != srv) {
            m_pendingState.vsTextures.at(slot) = srv;
            m_pendingState.vsDirtyTextureSlots.emplace_back((uint32_t)slot);
        }
    }

    void DX11Context::DrawPrimitive(D3D11_PRIMITIVE_TOPOLOGY primitiveType, uint32_t startVertex, uint32_t numVertices, bool indexed) {

        if (m_pendingState.pixelShaderHandle != 0 && m_currentState.pixelShaderHandle != m_pendingState.pixelShaderHandle) {
            m_devcon->PSSetShader(m_pendingState.pixelShader, 0, 0);
            if (m_pendingState.psCBuffer)
                m_devcon->PSSetConstantBuffers(
                    0, static_cast<UINT>(1), &m_pendingState.psCBuffer);
        }

        if (m_pendingState.vertexShaderHandle != 0 && m_currentState.vertexShaderHandle != m_pendingState.vertexShaderHandle) {
            m_devcon->VSSetShader(m_pendingState.vertexShader, 0, 0);
            if (m_pendingState.vsCBuffer)
                m_devcon->VSSetConstantBuffers(
                    0, static_cast<UINT>(1), &m_pendingState.vsCBuffer);
        }
        /*
        if (m_pendingState.vertexShaderHandle != 0 && m_pendingState.vsCBuffer)
            UpdateConstantBuffer(m_pendingState.vsCBuffer, m_pendingState.vsCBufferDirtySlots);

        if (m_pendingState.pixelShaderHandle != 0 && m_pendingState.psCBuffer)
            UpdateConstantBuffer(m_pendingState.psCBuffer, m_pendingState.psCBufferDirtySlots);
            */
        if (m_pendingState.inputLayoutHandle != 0 && m_currentState.inputLayoutHandle != m_pendingState.inputLayoutHandle)
            m_devcon->IASetInputLayout(m_pendingState.inputLayout);

        if (m_pendingState.vertexBufferHandle != 0 && m_currentState.vertexBufferHandle != m_pendingState.vertexBufferHandle) {
            uint32_t offset = 0;
            uint32_t stride = static_cast<UINT>(m_pendingState.inputLayoutStride);
            m_devcon->IASetVertexBuffers(0, 1, &m_pendingState.vertexBuffer, &stride, &offset);
        }

        if (m_pendingState.vsDirtyTextureSlots.size()) {
            // todo: batch these calls
            for (uint32_t x = 0; x < m_pendingState.vsDirtyTextureSlots.size(); ++x) {
                m_devcon->VSSetShaderResources((uint32_t)m_pendingState.vsDirtyTextureSlots[x], 1, &m_pendingState.vsTextures.at(x));
                m_devcon->VSSetSamplers((uint32_t)m_pendingState.vsDirtyTextureSlots[x], 1, &m_defaultSampler);
            }
        }

        if (m_pendingState.psDirtyTextureSlots.size()) {
            for (uint32_t x = 0; x < m_pendingState.psDirtyTextureSlots.size(); ++x) {
                m_devcon->PSSetShaderResources((uint32_t)m_pendingState.psDirtyTextureSlots[x], 1, &m_pendingState.psTextures.at(x));
                m_devcon->PSSetSamplers((uint32_t)m_pendingState.psDirtyTextureSlots[x], 1, &m_defaultSampler);
            }
        }

        // todo: format?
        if (m_pendingState.indexBufferHandle != 0 && m_currentState.indexBufferHandle != m_pendingState.indexBufferHandle)
            m_devcon->IASetIndexBuffer(m_pendingState.indexBuffer, DXGI_FORMAT_R32_UINT, 0);

        m_pendingState.primitiveType = primitiveType;

        if (m_currentState.primitiveType != primitiveType)
            m_devcon->IASetPrimitiveTopology(primitiveType);

        if (m_pendingState.blendStateHash != 0 && m_currentState.blendStateHash != m_pendingState.blendStateHash) {
            float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
            UINT sampleMask = 0xffffffff;
            m_devcon->OMSetBlendState(m_pendingState.blendState, blendFactor, sampleMask);
        }

        if (m_pendingState.rasterStateHash != 0 && m_currentState.rasterStateHash != m_pendingState.rasterStateHash)
            m_devcon->RSSetState(m_pendingState.rasterState);

        if (m_pendingState.depthStateHash != 0 && m_currentState.depthStateHash != m_pendingState.depthStateHash)
            m_devcon->OMSetDepthStencilState(m_pendingState.depthState, 1);

        // We use SWAP_FLIP_SEQUENTIAL with 11.3, which apparently needs this call everytime....ugh
#ifdef DX11_3_API
        m_devcon->OMSetRenderTargets(1, m_renderTarget.GetAddressOf(), m_depthStencilView.Get());
#endif
        if (indexed)
            m_devcon->DrawIndexed(numVertices, startVertex, 0);
        else
            m_devcon->Draw(numVertices, startVertex);

        //cleanup state before set
        m_pendingState.psCBufferDirtySlots.clear();
        m_pendingState.psDirtyTextureSlots.clear();
        m_pendingState.vsCBufferDirtySlots.clear();
        m_pendingState.vsDirtyTextureSlots.clear();
        m_currentState = m_pendingState;
    }

    DX11Context::~DX11Context() {
        m_devcon.Reset();
    }
}