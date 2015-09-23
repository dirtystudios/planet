#pragma once
#include <d3dcompiler.h>
#include <d3dcompiler.inl>
#include <d3d11.h>
#include <vector>
#include <unordered_map>

namespace graphics {
    typedef uint32_t InputLayoutCacheHandle;

    class DX11InputLayoutCache {
    private:
        struct DX11InputLayout {
            std::vector<D3D11_INPUT_ELEMENT_DESC> elements;
        };

        std::unordered_map<InputLayoutCacheHandle, DX11InputLayout> m_inputLayouts;

    public:
        InputLayoutCacheHandle InsertInputLayout(ID3DBlob* shaderBlob);
        const D3D11_INPUT_ELEMENT_DESC* GetInputLayoutData(InputLayoutCacheHandle handle);
        size_t GetInputLayoutSize(InputLayoutCacheHandle handle);
        void RemoveInputLayout(InputLayoutCacheHandle handle);

    private: 
        std::vector<D3D11_INPUT_ELEMENT_DESC> GenerateInputLayout(ID3DBlob* pShaderBlob);
    };
}