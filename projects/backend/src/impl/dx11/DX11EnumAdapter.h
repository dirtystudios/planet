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

namespace gfx {
using namespace Microsoft::WRL;
    static DXGI_FORMAT VertexAttributeTypeDX11[(uint32_t)VertexAttributeType::Count] = {
        DXGI_FORMAT_R32_FLOAT,
        DXGI_FORMAT_R32G32_FLOAT,
        DXGI_FORMAT_R32G32B32_FLOAT,
        DXGI_FORMAT_R32G32B32A32_FLOAT,
    };

    static D3D11_MAP MapAccessDX11[(uint32_t)BufferAccess::Count] = {
        D3D11_MAP_READ,               // Read
        D3D11_MAP_WRITE_DISCARD,      // Write
        D3D11_MAP_WRITE_NO_OVERWRITE, // WriteInit
        D3D11_MAP_READ_WRITE,         // ReadWrite
    };

    static DXGI_FORMAT PixelFormatDX11[(uint32_t) PixelFormat::Count] = {
        DXGI_FORMAT_R8_UNORM,           // R8Unorm
        DXGI_FORMAT_R8G8B8A8_UNORM,     // RGB8Unorm, Converted before load
        DXGI_FORMAT_R8G8B8A8_UNORM,     // RGBA8Unorm
        DXGI_FORMAT_R8_UINT,            // R8Uint
        DXGI_FORMAT_R32_FLOAT,          // R32Float
        DXGI_FORMAT_R32G32B32_FLOAT,    // RGB32Float
        DXGI_FORMAT_R32G32B32A32_FLOAT, // RGBA32Float

    };

    static D3D11_PRIMITIVE_TOPOLOGY PrimitiveTypeDX11[(uint32_t)PrimitiveType::Count] = {
        D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,    // triangles!
        D3D_PRIMITIVE_TOPOLOGY_LINESTRIP,       // LineStrip
        D3D_PRIMITIVE_TOPOLOGY_LINELIST,        // lines
    };

    static D3D11_BLEND_OP BlendModeDX11[(uint32_t)BlendMode::Count] = {
        D3D11_BLEND_OP_ADD,
        D3D11_BLEND_OP_SUBTRACT,
        D3D11_BLEND_OP_REV_SUBTRACT,
        D3D11_BLEND_OP_MIN,
        D3D11_BLEND_OP_MAX,
    };

    static D3D11_BLEND BlendFuncDX11[(uint32_t)BlendFunc::Count]{
        D3D11_BLEND_ZERO,
        D3D11_BLEND_ONE,
        D3D11_BLEND_SRC_COLOR,
        D3D11_BLEND_INV_SRC_COLOR,
        D3D11_BLEND_SRC_ALPHA,
        D3D11_BLEND_INV_SRC_ALPHA,
        D3D11_BLEND_DEST_ALPHA,
        D3D11_BLEND_INV_DEST_ALPHA,
        D3D11_BLEND_DEST_COLOR,
        D3D11_BLEND_INV_DEST_COLOR,
        D3D11_BLEND_SRC_ALPHA_SAT,
        D3D11_BLEND_SRC1_COLOR,
        D3D11_BLEND_INV_SRC1_COLOR,
        D3D11_BLEND_SRC1_ALPHA,
        D3D11_BLEND_INV_SRC1_ALPHA
    };

    static D3D11_FILL_MODE FillModeDX11[(uint32_t)FillMode::Count]{
        D3D11_FILL_WIREFRAME,
        D3D11_FILL_SOLID
    };

    static D3D11_CULL_MODE CullModeDX11[(uint32_t)CullMode::Count]{
        D3D11_CULL_NONE,
        D3D11_CULL_FRONT,
        D3D11_CULL_BACK
    };

    static D3D11_DEPTH_WRITE_MASK DepthWriteMaskDX11[(uint32_t)DepthWriteMask::Count]{
        D3D11_DEPTH_WRITE_MASK_ZERO,
        D3D11_DEPTH_WRITE_MASK_ALL
    };

    static D3D11_COMPARISON_FUNC DepthFuncDX11[(uint32_t)DepthFunc::Count]{
        D3D11_COMPARISON_NEVER,
        D3D11_COMPARISON_LESS,
        D3D11_COMPARISON_EQUAL,
        D3D11_COMPARISON_LESS_EQUAL,
        D3D11_COMPARISON_GREATER,
        D3D11_COMPARISON_NOT_EQUAL,
        D3D11_COMPARISON_GREATER_EQUAL,
        D3D11_COMPARISON_ALWAYS
    };
}
