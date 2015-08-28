#include "DX11ConstantBufferCache.h"
#include <wrl.h>
#include "../../Log.h"

#include <d3dcompiler.h>
#include <d3dcompiler.inl>

using namespace Microsoft::WRL;

namespace graphics {
    bool DX11ConstantBufferCache::InsertConstantBuffer(ID3DBlob* shaderBlob, ConstantBufferCacheHandle handle) {
        ComPtr<ID3D11ShaderReflection> shaderReflection;
        HRESULT hr;

        hr = D3D11Reflect(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), &shaderReflection);


        D3D11_SHADER_DESC shaderDesc;
        hr = shaderReflection->GetDesc(&shaderDesc);
        if (FAILED(hr)) {
            LOG_E("DX11Render: GenerateConstantBuffer: Failed to get shaderDesc. HR: 0x%x", hr);
            return false;
        }

        //TODO: Support more than 1 constantbuffer
        //for (uint32_t x = 0; x < shaderDesc.ConstantBuffers; ++x) {
        if (shaderDesc.ConstantBuffers > 0) {
            if (shaderDesc.ConstantBuffers > 1) {
                LOG_E("DX11Render: More than 1 constant buffer detected in shader. Only taking first.");
            }
            // not COM interface, yey!
            ID3D11ShaderReflectionConstantBuffer* cBufferReflection;
            cBufferReflection = shaderReflection->GetConstantBufferByIndex(0);
            if (!cBufferReflection) {
                LOG_E("DX11Render: Null cBufferDesc...");
                return false;
            }
            D3D11_SHADER_BUFFER_DESC cBufferDesc;
            hr = cBufferReflection->GetDesc(&cBufferDesc);
            if (FAILED(hr)) {
                LOG_E("DX11Render: Failed to get cBufferDesc. HR: 0x%x", hr);
                return false;
            }
            if (cBufferDesc.Type != D3D_CT_CBUFFER) {
                LOG_E("DX11Render: Expected cBuffer, got type: %d.", cBufferDesc.Type);
                return false;
            }
            std::unique_ptr<CBufferDescriptor> cBufferDescCache(new CBufferDescriptor(cBufferDesc.Size, cBufferDesc.Name, cBufferDesc.Variables));

            for (uint32_t v = 0; v < cBufferDesc.Variables; ++v) {
                // not COM interface, double yey!
                ID3D11ShaderReflectionVariable* cBufferVar = cBufferReflection->GetVariableByIndex(v);
                D3D11_SHADER_VARIABLE_DESC cBufferVarDesc;
                hr = cBufferVar->GetDesc(&cBufferVarDesc);
                if (FAILED(hr)) {
                    LOG_E("DX11Render: Failed to get cbufferVarDesc. HR: 0x%x", hr);
                    return false;
                }
                CBufferVariable var = CBufferVariable(cBufferVarDesc.StartOffset, cBufferVarDesc.Size);
                cBufferDescCache->details.insert(std::make_pair(cBufferVarDesc.Name, var));
            }

            m_CBuffers.insert(std::make_pair(handle, std::move(cBufferDescCache)));
            return true;
        }
        return false;
    }
    void DX11ConstantBufferCache::UpdateConstantBuffer(ConstantBufferCacheHandle handle, ParamType paramType, const char *pparamName, void* data) {
        auto it = m_CBuffers.find(handle);
        if (it == m_CBuffers.end()) {
            LOG_E("DX11Render: Invalid CBuffer handle given: %d ", handle);
            return;
        }

        std::string paramName(pparamName);
        auto it2 = it->second->details.find(paramName);
        if (it2 == it->second->details.end()) {
            LOG_E("DX11Render: Update CB Invalid paramName. Given: %s", paramName);
            return;
        }

        uint32_t paramSize = it2->second.size;
        if (SizeofParam(paramType) != paramSize) {
            LOG_E("DX11Render: Size mismatch for CB Update. Expected: %d, Given: %d", paramSize, SizeofParam(paramType));
            return;
        }

        it->second->UpdateBufferData(&it2->second, data);
    }
    uint32_t DX11ConstantBufferCache::GetConstantBufferSize(ConstantBufferCacheHandle handle) {
        auto it = m_CBuffers.find(handle);
        if (it == m_CBuffers.end()) {
            LOG_E("DX11Render: Invalid CBuffer handle given: %d ", handle);
            return 0;
        }
        return it->second->totalSize;
    }

    void* DX11ConstantBufferCache::GetConstantBufferData(ConstantBufferCacheHandle handle) {
        auto it = m_CBuffers.find(handle);
        if (it == m_CBuffers.end()) {
            LOG_E("DX11Render: Invalid CBuffer handle given: %d ", handle);
            return 0;
        }
        return it->second->bufferData;
    }
    void DX11ConstantBufferCache::RemoveConstantBuffer(ConstantBufferCacheHandle handle) {
        auto it = m_CBuffers.find(handle);
        if (it == m_CBuffers.end()) {
            // w/e its gone
            return;
        }
        // TODO: make sure this calls destructor in cbufferdescriptor
        m_CBuffers.erase(handle);
    }
}