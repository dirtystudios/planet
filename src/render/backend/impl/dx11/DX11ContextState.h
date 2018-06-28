#pragma once

#include "DX11EnumAdapter.h"

#include <unordered_map>
#include <unordered_set>
#include <array>

namespace gfx {
    struct DX11ContextState {
        ID3D11Buffer* indexBuffer{ nullptr };

        ID3D11VertexShader* vertexShader{ nullptr };

        std::unordered_map<uint32_t, ID3D11Buffer*> vsCBuffers;
        std::unordered_set<uint32_t> vsCBufferDirtySlots;

        std::unordered_map<uint32_t, ID3D11ShaderResourceView*> vsTextures;
        std::unordered_map<uint32_t, ID3D11SamplerState*> vsSamplers;
        std::unordered_set<uint32_t> vsDirtyTextureSlots;

        std::unordered_map<uint32_t, ID3D11Buffer*> psCBuffers;
        std::unordered_set<uint32_t> psCBufferDirtySlots;

        std::unordered_map<uint32_t, ID3D11ShaderResourceView*> psTextures;
        std::unordered_map<uint32_t, ID3D11SamplerState*> psSamplers;
        std::unordered_set<uint32_t> psDirtyTextureSlots;

        std::array<ID3D11RenderTargetView*, 8> rtvs{};
        uint8_t rtv_count{ 0 };

        ID3D11DepthStencilView* dsv;

        ID3D11PixelShader* pixelShader{ nullptr };

        ID3D11InputLayout* inputLayout{ nullptr };
        uint32_t inputLayoutStride;

        ID3D11Buffer* vertexBuffer{ nullptr };
        size_t vertexBufferStride{ 0 };

        ID3D11BlendState* blendState{ nullptr };
        ID3D11RasterizerState* rasterState{ nullptr };
        ID3D11DepthStencilState* depthState{ nullptr };

        size_t vphash{0};
        D3D11_VIEWPORT vp;

        D3D11_PRIMITIVE_TOPOLOGY primitiveType{ D3D_PRIMITIVE_TOPOLOGY_UNDEFINED };
    };
}
