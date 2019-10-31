#pragma once

#include "BufferAccessFlags.h"
#include "BufferUsageFlags.h"
#include "PrimitiveType.h"
#include "ShaderType.h"
#include "RenderPassInfo.h"
#include "TextureUsage.h"
#include "Resource.h"

#include "d3dx12Residency.h"

#include <wrl.h>
#include <d3d12.h>

namespace gfx {

    struct BufferDX12 : public Resource {
        Microsoft::WRL::ComPtr<ID3D12Resource> buffer;
        Microsoft::WRL::ComPtr<ID3D12Resource> cpubuffer; // used for uploadbuffer when needed
        D3D12_CPU_DESCRIPTOR_HANDLE cbv;
        D3D12_CPU_DESCRIPTOR_HANDLE uav;
        D3D12_RESOURCE_STATES currentState;
        BufferAccessFlags accessFlags;
        BufferUsageFlags usageFlags;
        size_t size;
        D3DX12Residency::ManagedObject trackingHandle;
        uint64_t copyFenceValue;
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
        D3D12_CPU_DESCRIPTOR_HANDLE sampler;
        D3D12_CPU_DESCRIPTOR_HANDLE srv;
        D3D12_CPU_DESCRIPTOR_HANDLE uav;
        D3D12_CPU_DESCRIPTOR_HANDLE rtv;
        D3D12_CPU_DESCRIPTOR_HANDLE dsv;
        D3D12_RESOURCE_STATES currentState;
        TextureUsageFlags usage;
        DXGI_FORMAT format;
        PixelFormat requestedFormat;
        uint32_t width;
        uint32_t height;
        uint64_t size;
        D3DX12Residency::ManagedObject trackingHandle;
        uint64_t copyFenceValue;
    };

    struct RenderPassDX12 : public Resource {
        RenderPassInfo info;
    };
}
