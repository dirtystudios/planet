#pragma once

#include "DX11Structs.h"

#include <unordered_map>
#include <vector>

namespace gfx {
    struct DX11ContextState {
        size_t indexBufferHandle;
        ID3D11Buffer* indexBuffer;

        size_t vertexShaderHandle;
        ID3D11VertexShader* vertexShader;

        std::unordered_map<uint32_t, ID3D11Buffer*> vsCBuffers;
        std::vector<uint32_t> vsCBufferDirtySlots;

        std::unordered_map<uint32_t, ID3D11ShaderResourceView*> vsTextures;
        std::vector<uint32_t> vsDirtyTextureSlots;

        std::unordered_map<uint32_t, ID3D11Buffer*> psCBuffers;
        std::vector<uint32_t> psCBufferDirtySlots;

        std::unordered_map<uint32_t, ID3D11ShaderResourceView*> psTextures;
        std::vector<uint32_t> psDirtyTextureSlots;

        size_t pixelShaderHandle;
        ID3D11PixelShader* pixelShader;

        size_t inputLayoutHandle;
        ID3D11InputLayout* inputLayout;
        uint32_t inputLayoutStride;

        size_t vertexBufferHandle;
        ID3D11Buffer* vertexBuffer;
        size_t vertexBufferStride;

        size_t blendStateHash;
        ID3D11BlendState* blendState;

        size_t rasterStateHash;
        ID3D11RasterizerState* rasterState;

        size_t depthStateHash;
        ID3D11DepthStencilState* depthState;

        D3D11_PRIMITIVE_TOPOLOGY primitiveType;

        DX11ContextState()
            : vertexShaderHandle(0),
            vertexShader(0),
            pixelShaderHandle(0),
            pixelShader(0),
            inputLayoutHandle(0),
            inputLayout(0),
            blendState(0),
            blendStateHash(0),
            rasterState(0),
            rasterStateHash(0),
            depthState(0),
            depthStateHash(0),
            primitiveType(D3D_PRIMITIVE_TOPOLOGY_UNDEFINED) {}
    };
}
