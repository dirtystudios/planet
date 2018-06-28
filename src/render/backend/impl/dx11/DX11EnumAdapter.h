#pragma once

#include <d3d11.h>
#include <vector>

#include <wrl.h>

#include "BufferAccess.h"
#include "BufferUsage.h"
#include "ParamType.h"
#include "BlendMode.h"
#include "CullMode.h"
#include "DepthWriteMask.h"
#include "FillMode.h"
#include "BlendFunc.h"
#include "BlendState.h"
#include "WindingOrder.h"
#include "PixelFormat.h"
#include "Helpers.h"
#include "PrimitiveType.h"
#include "RasterState.h"
#include "DepthFunc.h"
#include "DepthState.h"
#include "VertexLayoutDesc.h"
#include "ShaderType.h"
#include "TextureUsage.h"

#define SafeGet(id, idx) id[(uint32_t)idx]

namespace gfx {
    static DXGI_FORMAT GetVertexAttributeFormatDX11(VertexAttributeType type, VertexAttributeStorage storage) {
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

    static UINT GetBindFlagsDX11(TextureUsageFlags flags, PixelFormat format) {
        UINT rtn = 0;
        uint32_t check = (uint32_t)flags;

        if (flags & TextureUsageFlags::RenderTarget) {
            if (format == PixelFormat::Depth32Float || format == PixelFormat::Depth32FloatStencil8)
                rtn |= D3D11_BIND_DEPTH_STENCIL;
            else
                rtn |= D3D11_BIND_RENDER_TARGET;

            check &= ~(uint32_t)TextureUsageFlags::RenderTarget;
        }

        if ((flags & TextureUsageFlags::ShaderRead) || (flags & TextureUsageFlags::ShaderWrite)) {
            rtn |= D3D11_BIND_SHADER_RESOURCE;
            check &= ~((uint32_t)TextureUsageFlags::ShaderRead | (uint32_t)TextureUsageFlags::ShaderWrite);
        }
        dg_assert(check == 0, "unhandled usage flag");
        return rtn;
    }

    static D3D11_MAP MapAccessDX11[] = {
        D3D11_MAP_READ,               // Read
        D3D11_MAP_WRITE_DISCARD,      // Write
        D3D11_MAP_WRITE_NO_OVERWRITE, // WriteInit
        D3D11_MAP_READ_WRITE,         // ReadWrite
    };
    static_assert(sizeof(MapAccessDX11) / sizeof(D3D11_MAP) == (uint32_t)BufferAccess::Count, "");

    static DXGI_FORMAT PixelFormatDX11[] = {
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

    static_assert(sizeof(PixelFormatDX11) / sizeof(DXGI_FORMAT) == (uint32_t)PixelFormat::Count, "");

    static D3D11_PRIMITIVE_TOPOLOGY PrimitiveTypeDX11[] = {
        D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,    // triangles!
        D3D_PRIMITIVE_TOPOLOGY_LINESTRIP,       // LineStrip
        D3D_PRIMITIVE_TOPOLOGY_LINELIST,        // lines
    };
    static_assert(sizeof(PrimitiveTypeDX11) / sizeof(D3D11_PRIMITIVE_TOPOLOGY) == (uint32_t)PrimitiveType::Count, "");

    static D3D11_BLEND_OP BlendModeDX11[] = {
        D3D11_BLEND_OP_ADD,
        D3D11_BLEND_OP_SUBTRACT,
        D3D11_BLEND_OP_REV_SUBTRACT,
        D3D11_BLEND_OP_MIN,
        D3D11_BLEND_OP_MAX,
    };
    static_assert(sizeof(BlendModeDX11) / sizeof(D3D11_BLEND_OP) == (uint32_t)BlendMode::Count, "");

    static D3D11_BLEND BlendFuncDX11[] {
        D3D11_BLEND_ZERO,
        D3D11_BLEND_ONE,

        D3D11_BLEND_SRC_COLOR,
        D3D11_BLEND_INV_SRC_COLOR,

        D3D11_BLEND_SRC_ALPHA,
        D3D11_BLEND_INV_SRC_ALPHA,
        D3D11_BLEND_DEST_ALPHA,
        D3D11_BLEND_INV_DEST_ALPHA,

        // slightly wrong
        D3D11_BLEND_BLEND_FACTOR,
        D3D11_BLEND_INV_BLEND_FACTOR,
        D3D11_BLEND_BLEND_FACTOR,
        D3D11_BLEND_INV_BLEND_FACTOR,

        D3D11_BLEND_SRC_ALPHA_SAT,

        D3D11_BLEND_SRC1_COLOR,
        D3D11_BLEND_INV_SRC1_COLOR,
        D3D11_BLEND_SRC1_ALPHA,
        D3D11_BLEND_INV_SRC1_ALPHA
    };
    static_assert(sizeof(BlendFuncDX11) / sizeof(D3D11_BLEND) == (uint32_t)BlendFunc::Count, "");

    static D3D11_FILL_MODE FillModeDX11[]{
        D3D11_FILL_WIREFRAME,
        D3D11_FILL_SOLID
    };
    static_assert(sizeof(FillModeDX11) / sizeof(D3D11_FILL_MODE) == (uint32_t)FillMode::Count, "");

    static D3D11_CULL_MODE CullModeDX11[]{
        D3D11_CULL_NONE,
        D3D11_CULL_FRONT,
        D3D11_CULL_BACK
    };
    static_assert(sizeof(CullModeDX11) / sizeof(D3D11_CULL_MODE) == (uint32_t)CullMode::Count, "");

    static D3D11_DEPTH_WRITE_MASK DepthWriteMaskDX11[]{
        D3D11_DEPTH_WRITE_MASK_ZERO,
        D3D11_DEPTH_WRITE_MASK_ALL
    };
    static_assert(sizeof(DepthWriteMaskDX11) / sizeof(D3D11_DEPTH_WRITE_MASK) == (uint32_t)DepthWriteMask::Count, "");

    static D3D11_COMPARISON_FUNC DepthFuncDX11[]{
        D3D11_COMPARISON_NEVER,
        D3D11_COMPARISON_LESS,
        D3D11_COMPARISON_EQUAL,
        D3D11_COMPARISON_LESS_EQUAL,
        D3D11_COMPARISON_GREATER,
        D3D11_COMPARISON_NOT_EQUAL,
        D3D11_COMPARISON_GREATER_EQUAL,
        D3D11_COMPARISON_ALWAYS
    };
    static_assert(sizeof(DepthFuncDX11) / sizeof(D3D11_COMPARISON_FUNC) == (uint32_t)DepthFunc::Count, "");
}
