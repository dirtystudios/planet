#pragma once

#include "BufferAccessFlags.h"
#include "BufferUsageFlags.h"
#include "PrimitiveType.h"
#include "ShaderType.h"
#include "RenderPassInfo.h"

#include <wrl.h>
#include <d3d12.h>

namespace gfx {

    struct BufferDX12 : public Resource {
        Microsoft::WRL::ComPtr<ID3D12Resource> buffer;
        BufferAccessFlags accessFlags;
        BufferUsageFlags usageFlags;
        size_t size;
    };

    struct PipelineStateDX12 : public Resource {
        Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
        VertexLayoutId vertexLayoutId;
        PrimitiveType primitiveType;
    };

    struct ShaderDX12 : public Resource {
        Microsoft::WRL::ComPtr<ID3DBlob> blob;
        ShaderType shaderType;
    };

    struct InputLayoutDX12 : public Resource {
        std::vector<D3D12_INPUT_ELEMENT_DESC> elements;
        uint32_t stride;
    };

    struct TextureDX12 : public Resource {
        Microsoft::WRL::ComPtr<ID3D12Resource> resource;
    };

    struct RenderPassDX12 : public Resource {
        RenderPassInfo info;
    };
}
