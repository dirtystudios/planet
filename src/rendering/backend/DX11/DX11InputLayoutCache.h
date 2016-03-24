#pragma once
#include <d3dcompiler.h>
#include <d3dcompiler.inl>
#include <d3d11.h>
#include <vector>
#include <unordered_map>
#include <memory>

namespace graphics {
    typedef uint32_t InputLayoutCacheHandle;

    class DX11InputLayoutCache {
    private:
        struct DX11InputLayout {
            std::vector<D3D11_INPUT_ELEMENT_DESC> elements;
            uint32_t numRefs;
        };

        std::unordered_map<InputLayoutCacheHandle, DX11InputLayout> m_inputLayouts;

		// This is to cache semanticstrings, apparently sometimes we lose the reference on windows 7
		std::vector<std::unique_ptr<const char[]>> m_semanticNameCache;

    public:
        InputLayoutCacheHandle InsertInputLayout(ID3DBlob* shaderBlob);
        const D3D11_INPUT_ELEMENT_DESC* GetInputLayoutData(InputLayoutCacheHandle handle);
        size_t GetInputLayoutSize(InputLayoutCacheHandle handle);
        void RemoveInputLayout(InputLayoutCacheHandle handle);

    private: 
        std::vector<D3D11_INPUT_ELEMENT_DESC> GenerateInputLayout(ID3DBlob* pShaderBlob);
    };
}