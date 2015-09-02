#pragma once

#include <string>
#include <cstring>
#include <unordered_map>
#include <vector>
#include <memory>
#include <d3dcompiler.h>
#include <d3dcompiler.inl>
#include <wrl.h>

#include "../ParamType.h"
#include "../../Log.h"

using namespace Microsoft::WRL;

// Random Static Helper things for CBuffers

// TODO: dont use mallocs and 'new' here

namespace graphics {
    namespace dx11 {
        struct CBufferVariable
        {
            size_t  offset;
            size_t  size;
            CBufferVariable(size_t m_offset, size_t m_size)
                : offset(m_offset), size(m_size) {}
        };

        class CBufferDescriptor
        {
        public:
            size_t totalSize;
            std::string name;
            // string holds name of variable
            std::unordered_map<std::string, CBufferVariable> details;
            size_t numVars;
            void *bufferData;

            CBufferDescriptor() {};

            void ResetDescriptor(size_t p_size, std::string p_name, size_t p_numVars) {
                totalSize = p_size;
                name = p_name;
                numVars = p_numVars;
                bufferData = calloc(totalSize / 16, 16);
            }

            ~CBufferDescriptor() {
                if (bufferData)
                    free(bufferData);
            }

            //Note, this isn't bounds/size checked, do it elsewhere
            void UpdateBufferData(const CBufferVariable *var, void* value) {
                void *data = (char *)bufferData + var->offset;
                memcpy(data, value, var->size);
            }

            // Not dealing with this copy pointer bs
            CBufferDescriptor & operator=(const CBufferDescriptor&) = delete;
            CBufferDescriptor(const CBufferDescriptor &other) = delete;
        };

        // Returns array, numCBuffers is out
        CBufferDescriptor* GenerateConstantBuffer(ID3DBlob* shaderBlob, size_t* numCBuffers) {
            ComPtr<ID3D11ShaderReflection> shaderReflection;
            HRESULT hr;
            *numCBuffers = 0;

#ifdef D3D11Reflect
            hr = D3D11Reflect(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), &shaderReflection);
#else
            hr = D3D12Reflect(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), &shaderReflection);
#endif
            D3D11_SHADER_DESC shaderDesc;
            hr = shaderReflection->GetDesc(&shaderDesc);
            if (FAILED(hr)) {
                LOG_E("DX11Render: GenerateConstantBuffer: Failed to get shaderDesc. HR: 0x%x", hr);
                return 0;
            }

            //todo: dont do this
            CBufferDescriptor *cBufferDescriptorArray = new CBufferDescriptor[shaderDesc.ConstantBuffers]();

            for (uint32_t x = 0; x < shaderDesc.ConstantBuffers; ++x) {
                ID3D11ShaderReflectionConstantBuffer* cBufferReflection;
                cBufferReflection = shaderReflection->GetConstantBufferByIndex(0);
                if (!cBufferReflection) {
                    LOG_E("DX11Render: Null cBufferDesc...");
                    delete[] cBufferDescriptorArray;
                    return 0;
                }

                D3D11_SHADER_BUFFER_DESC cBufferDesc;
                hr = cBufferReflection->GetDesc(&cBufferDesc);
                if (FAILED(hr)) {
                    LOG_E("DX11Render: Failed to get cBufferDesc. HR: 0x%x", hr);
                    delete[] cBufferDescriptorArray;
                    return 0;
                }

                if (cBufferDesc.Type != D3D_CT_CBUFFER) {
                    LOG_E("DX11Render: Expected cBuffer, got type: %d.", cBufferDesc.Type);
                    delete[] cBufferDescriptorArray;
                    return 0;
                }
                cBufferDescriptorArray[x].ResetDescriptor(cBufferDesc.Size, cBufferDesc.Name, cBufferDesc.Variables);

                for (uint32_t v = 0; v < cBufferDesc.Variables; ++v) {
                    ID3D11ShaderReflectionVariable* cBufferVar = cBufferReflection->GetVariableByIndex(v);
                    D3D11_SHADER_VARIABLE_DESC cBufferVarDesc;
                    hr = cBufferVar->GetDesc(&cBufferVarDesc);
                    if (FAILED(hr)) {
                        LOG_E("DX11Render: Failed to get cbufferVarDesc. HR: 0x%x", hr);
                        delete[] cBufferDescriptorArray;
                        return 0;
                    }

                    CBufferVariable var = CBufferVariable(cBufferVarDesc.StartOffset, cBufferVarDesc.Size);
                    cBufferDescriptorArray[x].details.insert(std::make_pair(cBufferVarDesc.Name, var));
                }
            }
            *numCBuffers = shaderDesc.ConstantBuffers;
            return cBufferDescriptorArray;
        }
    }
}