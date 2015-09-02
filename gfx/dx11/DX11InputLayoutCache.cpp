#include "DX11InputLayoutCache.h"
#include <wrl.h>

#include "../../Log.h"

using namespace Microsoft::WRL;

namespace graphics {
    InputLayoutCacheHandle DX11InputLayoutCache::InsertInputLayout(ID3DBlob* shaderBlob) {
        std::vector<D3D11_INPUT_ELEMENT_DESC> ieds = GenerateInputLayout(shaderBlob);

        if (ieds.size() == 0) {
            // either invalid, or there's really no input layout specified
            return 0;
        }

        //todo: hash stuff
        DX11InputLayout inputLayout;
        inputLayout.elements = ieds;

        uint32_t handle = GenerateHandle();
        m_inputLayouts.insert(std::make_pair(handle, inputLayout));

        return handle;
    }

    const D3D11_INPUT_ELEMENT_DESC* DX11InputLayoutCache::GetInputLayoutData(InputLayoutCacheHandle handle) {
        auto it = m_inputLayouts.find(handle);
        if (it == m_inputLayouts.end()) {
            LOG_E("DX11Render: Invalid InputLayout handle given: %d ", handle);
            return 0;
        }

        return &it->second.elements[0];
    }

    size_t DX11InputLayoutCache::GetInputLayoutSize(InputLayoutCacheHandle handle) {
        auto it = m_inputLayouts.find(handle);
        if (it == m_inputLayouts.end()) {
            LOG_E("DX11Render: Invalid InputLayout handle given: %d ", handle);
            return 0;
        }

        return it->second.elements.size();
    }

    void DX11InputLayoutCache::RemoveInputLayout(InputLayoutCacheHandle handle) {
        // todo, keep track of references, we wont want to always remove if its being used multiple times
        auto it = m_inputLayouts.find(handle);
        if (it == m_inputLayouts.end()) {
            return;
        }

        m_inputLayouts.erase(handle);
    }

    std::vector<D3D11_INPUT_ELEMENT_DESC> DX11InputLayoutCache::GenerateInputLayout(ID3DBlob* pShaderBlob) {
        ComPtr<ID3D11ShaderReflection> shaderReflection;
        HRESULT hr;
        std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
#ifdef D3D11Reflect
        hr = D3D11Reflect(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), &shaderReflection);
#else
        hr = D3D12Reflect(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), &shaderReflection);
#endif
        if (FAILED(hr)) {
            LOG_E("DX11Render: Failed to get shader reflection. HR: 0x%x", hr);
            return inputLayoutDesc;
        }

        D3D11_SHADER_DESC shaderDesc;
        hr = shaderReflection->GetDesc(&shaderDesc);
        if (FAILED(hr)) {
            LOG_E("DX11Render: Failed to get shaderDesc. HR: 0x%x", hr);
            return inputLayoutDesc;
        }

        for (uint32_t x = 0; x < shaderDesc.InputParameters; ++x) {
            D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
            hr = shaderReflection->GetInputParameterDesc(x, &paramDesc);
            if (FAILED(hr)) {
                LOG_E("DX11Render: Failed to get shader param desc. HR: 0x%x", hr);
                return inputLayoutDesc;
            }

            D3D11_INPUT_ELEMENT_DESC ied;
            ied.SemanticName = paramDesc.SemanticName;
            ied.SemanticIndex = paramDesc.SemanticIndex;
            ied.InputSlot = 0;
            ied.AlignedByteOffset = x == 0 ? 0 : D3D11_APPEND_ALIGNED_ELEMENT;
            ied.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
            ied.InstanceDataStepRate = 0;

            switch (paramDesc.Mask) {
            case 1:
                switch (paramDesc.ComponentType) {
                case D3D_REGISTER_COMPONENT_UINT32: ied.Format = DXGI_FORMAT_R32_UINT; break;
                case D3D_REGISTER_COMPONENT_SINT32: ied.Format = DXGI_FORMAT_R32_SINT; break;
                case D3D_REGISTER_COMPONENT_FLOAT32: ied.Format = DXGI_FORMAT_R32_FLOAT; break;
                default: LOG_E("DX11Render: Unknown ComponentType encounted!"); break;
                }
                break;
            case 3:
                switch (paramDesc.ComponentType) {
                case D3D_REGISTER_COMPONENT_UINT32: ied.Format = DXGI_FORMAT_R32G32_UINT; break;
                case D3D_REGISTER_COMPONENT_SINT32: ied.Format = DXGI_FORMAT_R32G32_SINT; break;
                case D3D_REGISTER_COMPONENT_FLOAT32: ied.Format = DXGI_FORMAT_R32G32_FLOAT; break;
                default: LOG_E("DX11Render: Unknown ComponentType encounted!"); break;
                }
                break;
            case 7:
                switch (paramDesc.ComponentType) {
                case D3D_REGISTER_COMPONENT_UINT32: ied.Format = DXGI_FORMAT_R32G32B32_UINT; break;
                case D3D_REGISTER_COMPONENT_SINT32: ied.Format = DXGI_FORMAT_R32G32B32_SINT; break;
                case D3D_REGISTER_COMPONENT_FLOAT32: ied.Format = DXGI_FORMAT_R32G32B32_FLOAT; break;
                default: LOG_E("DX11Render: Unknown ComponentType encounted!"); break;
                }
                break;
            case 15:
                switch (paramDesc.ComponentType) {
                case D3D_REGISTER_COMPONENT_UINT32: ied.Format = DXGI_FORMAT_R32G32B32A32_UINT; break;
                case D3D_REGISTER_COMPONENT_SINT32: ied.Format = DXGI_FORMAT_R32G32B32A32_SINT; break;
                case D3D_REGISTER_COMPONENT_FLOAT32: ied.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
                default: LOG_E("DX11Render: Unknown ComponentType encounted!"); break;
                }
                break;
            default:
                LOG_E("DX11Render: Unexpected paramdesc mask encountered!: %d", paramDesc.Mask);
                return inputLayoutDesc;
                break;
            }

            inputLayoutDesc.emplace_back(ied);
        }
        return inputLayoutDesc;
    }
}