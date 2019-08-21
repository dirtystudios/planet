#pragma once

#include "VertexLayoutDesc.h"
#include "FillMode.h"
#include "CullMode.h"
#include "BlendMode.h"
#include "BlendFunc.h"
#include "DepthWriteMask.h"
#include "DepthFunc.h"
#include "PixelFormat.h"

#include <d3d12.h>

#define SafeGet(id, idx) id[(uint32_t)idx]

namespace gfx {
    static DXGI_FORMAT PixelFormatDX12[(uint32_t)PixelFormat::Count] = {
        DXGI_FORMAT_R8_UNORM,           // R8Unorm
        DXGI_FORMAT_R8G8B8A8_UNORM,     // RGB8Unorm, Converted before load
        DXGI_FORMAT_R8G8B8A8_UNORM,     // RGBA8Unorm
        DXGI_FORMAT_R8_UINT,            // R8Uint
        DXGI_FORMAT_R32_FLOAT,          // R32Float
        DXGI_FORMAT_R32G32B32_FLOAT,    // RGB32Float
        DXGI_FORMAT_R32G32B32A32_FLOAT, // RGBA32Float

    };

    static DXGI_FORMAT VertexAttributeTypeDX12[(uint32_t)VertexAttributeType::Count] = {
        DXGI_FORMAT_R32_FLOAT,
        DXGI_FORMAT_R32G32_FLOAT,
        DXGI_FORMAT_R32G32B32_FLOAT,
        DXGI_FORMAT_R32G32B32A32_FLOAT,
    };

    static D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveTypeDX12[(uint32_t)PrimitiveType::Count] = {
        D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,    // triangles!
        D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,       // LineStrip
        D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,        // lines
    };

    static D3D12_FILL_MODE FillModeDX12[(uint32_t)FillMode::Count]{
        D3D12_FILL_MODE_WIREFRAME,
        D3D12_FILL_MODE_SOLID
    };

    static D3D12_CULL_MODE CullModeDX12[(uint32_t)CullMode::Count]{
        D3D12_CULL_MODE_NONE,
        D3D12_CULL_MODE_FRONT,
        D3D12_CULL_MODE_BACK
    };

    static D3D12_BLEND_OP BlendModeDX12[(uint32_t)BlendMode::Count] = {
        D3D12_BLEND_OP_ADD,
        D3D12_BLEND_OP_SUBTRACT,
        D3D12_BLEND_OP_REV_SUBTRACT,
        D3D12_BLEND_OP_MIN,
        D3D12_BLEND_OP_MAX,
    };

    static D3D12_BLEND BlendFuncDX12[(uint32_t)BlendFunc::Count]{
        D3D12_BLEND_ZERO,
        D3D12_BLEND_ONE,
        D3D12_BLEND_SRC_COLOR,
        D3D12_BLEND_INV_SRC_COLOR,
        D3D12_BLEND_SRC_ALPHA,
        D3D12_BLEND_INV_SRC_ALPHA,
        D3D12_BLEND_DEST_ALPHA,
        D3D12_BLEND_INV_DEST_ALPHA,
        D3D12_BLEND_DEST_COLOR,
        D3D12_BLEND_INV_DEST_COLOR,
        D3D12_BLEND_SRC_ALPHA_SAT,
        D3D12_BLEND_SRC1_COLOR,
        D3D12_BLEND_INV_SRC1_COLOR,
        D3D12_BLEND_SRC1_ALPHA,
        D3D12_BLEND_INV_SRC1_ALPHA
    };

    static D3D12_DEPTH_WRITE_MASK DepthWriteMaskDX12[(uint32_t)DepthWriteMask::Count]{
        D3D12_DEPTH_WRITE_MASK_ZERO,
        D3D12_DEPTH_WRITE_MASK_ALL
    };

    static D3D12_COMPARISON_FUNC DepthFuncDX12[(uint32_t)DepthFunc::Count]{
        D3D12_COMPARISON_FUNC_NEVER,
        D3D12_COMPARISON_FUNC_LESS,
        D3D12_COMPARISON_FUNC_EQUAL,
        D3D12_COMPARISON_FUNC_LESS_EQUAL,
        D3D12_COMPARISON_FUNC_GREATER,
        D3D12_COMPARISON_FUNC_NOT_EQUAL,
        D3D12_COMPARISON_FUNC_GREATER_EQUAL,
        D3D12_COMPARISON_FUNC_ALWAYS
    };
}