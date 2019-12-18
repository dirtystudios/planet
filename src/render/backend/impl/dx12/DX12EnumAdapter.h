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
    static DXGI_FORMAT PixelFormatDX12[] = {
        DXGI_FORMAT_UNKNOWN,              // Invalid
        DXGI_FORMAT_R8_UNORM,             // R8Unorm
        DXGI_FORMAT_R8G8B8A8_UNORM,       // RGB8Unorm, Converted before load
        DXGI_FORMAT_R8G8B8A8_UNORM,       // RGBA8Unorm
        DXGI_FORMAT_R8_UINT,              // R8Uint
        DXGI_FORMAT_R32_FLOAT,            // R32Float
        DXGI_FORMAT_R32G32B32_FLOAT,      // RGB32Float
        DXGI_FORMAT_R32G32B32A32_FLOAT,   // RGBA32Float
        DXGI_FORMAT_B8G8R8A8_UNORM,       // BGRAUnorm
        DXGI_FORMAT_D32_FLOAT,            // Depth32Float,
        DXGI_FORMAT_D32_FLOAT_S8X24_UINT, // Depth32Float8Stencil

    };
    static_assert(sizeof(PixelFormatDX12) / sizeof(DXGI_FORMAT) == (uint32_t)PixelFormat::Count, "");

    static DXGI_FORMAT GetVertexAttributeFormatDX12(VertexAttributeType type, VertexAttributeStorage storage) {
        switch (type) {
        case VertexAttributeType::Float: {
            switch (storage) {
            case VertexAttributeStorage::Float:
                return DXGI_FORMAT_R32_FLOAT;
            }
        }
        case VertexAttributeType::Float2: {
            switch (storage) {
            case VertexAttributeStorage::Float:
                return DXGI_FORMAT_R32G32_FLOAT;
            }
        }
        case VertexAttributeType::Float3: {
            switch (storage) {
            case VertexAttributeStorage::Float:
                return DXGI_FORMAT_R32G32B32_FLOAT;
            }
        }
        case VertexAttributeType::Float4: {
            switch (storage) {
            case VertexAttributeStorage::Float:
                return DXGI_FORMAT_R32G32B32A32_FLOAT;
            }
        }
        case VertexAttributeType::Int4: {
            switch (storage) {
            case VertexAttributeStorage::UInt32N:
                return DXGI_FORMAT_R32G32B32A32_UINT;
            }
        }
        default:
            dg_assert_fail("implement vertexattribute type for dx11.");
        }
        dg_assert_fail("implement vertexattribute type for dx11.");
        // silence warning
        return DXGI_FORMAT_R32_FLOAT;
    };

    static D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveTypeDX12[] = {
        D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,    // triangles!
        D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,       // LineStrip
        D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,        // lines
    };
    static_assert(sizeof(PrimitiveTypeDX12) / sizeof(D3D12_PRIMITIVE_TOPOLOGY_TYPE) == (uint32_t)PrimitiveType::Count, "");

    static D3D12_FILL_MODE FillModeDX12[]{
        D3D12_FILL_MODE_WIREFRAME,
        D3D12_FILL_MODE_SOLID
    };
    static_assert(sizeof(FillModeDX12) / sizeof(D3D12_FILL_MODE) == (uint32_t)FillMode::Count, "");

    static D3D12_CULL_MODE CullModeDX12[]{
        D3D12_CULL_MODE_NONE,
        D3D12_CULL_MODE_FRONT,
        D3D12_CULL_MODE_BACK
    };
    static_assert(sizeof(CullModeDX12) / sizeof(D3D12_CULL_MODE) == (uint32_t)CullMode::Count, "");

    static D3D12_BLEND_OP BlendModeDX12[] = {
        D3D12_BLEND_OP_ADD,
        D3D12_BLEND_OP_SUBTRACT,
        D3D12_BLEND_OP_REV_SUBTRACT,
        D3D12_BLEND_OP_MIN,
        D3D12_BLEND_OP_MAX,
    };
    static_assert(sizeof(BlendModeDX12) / sizeof(D3D12_BLEND_OP) == (uint32_t)BlendMode::Count, "");

    static D3D12_BLEND BlendFuncDX12[]{
        D3D12_BLEND_ZERO,
        D3D12_BLEND_ONE,

        D3D12_BLEND_SRC_COLOR,
        D3D12_BLEND_INV_SRC_COLOR,

        D3D12_BLEND_SRC_ALPHA,
        D3D12_BLEND_INV_SRC_ALPHA,
        D3D12_BLEND_DEST_ALPHA,
        D3D12_BLEND_INV_DEST_ALPHA,

        // slightly wrong
        D3D12_BLEND_BLEND_FACTOR,
        D3D12_BLEND_INV_BLEND_FACTOR,
        D3D12_BLEND_BLEND_FACTOR,
        D3D12_BLEND_INV_BLEND_FACTOR,

        D3D12_BLEND_SRC_ALPHA_SAT,

        D3D12_BLEND_SRC1_COLOR,
        D3D12_BLEND_INV_SRC1_COLOR,
        D3D12_BLEND_SRC1_ALPHA,
        D3D12_BLEND_INV_SRC1_ALPHA
    };
    static_assert(sizeof(BlendFuncDX12) / sizeof(D3D12_BLEND) == (uint32_t)BlendFunc::Count, "");

    static D3D12_DEPTH_WRITE_MASK DepthWriteMaskDX12[]{
        D3D12_DEPTH_WRITE_MASK_ZERO,
        D3D12_DEPTH_WRITE_MASK_ALL
    };
    static_assert(sizeof(DepthWriteMaskDX12) / sizeof(D3D12_DEPTH_WRITE_MASK) == (uint32_t)DepthWriteMask::Count, "");

    static D3D12_COMPARISON_FUNC DepthFuncDX12[]{
        D3D12_COMPARISON_FUNC_NEVER,
        D3D12_COMPARISON_FUNC_LESS,
        D3D12_COMPARISON_FUNC_EQUAL,
        D3D12_COMPARISON_FUNC_LESS_EQUAL,
        D3D12_COMPARISON_FUNC_GREATER,
        D3D12_COMPARISON_FUNC_NOT_EQUAL,
        D3D12_COMPARISON_FUNC_GREATER_EQUAL,
        D3D12_COMPARISON_FUNC_ALWAYS
    };
    static_assert(sizeof(DepthFuncDX12) / sizeof(D3D12_COMPARISON_FUNC) == (uint32_t)DepthFunc::Count, "");
}