#pragma once

#include <stdint.h>
#include <vector> 

namespace gfx {
enum class StateGroupIndex : uint16_t {
    VertexShader  = 0,
    PixelShader   = 1,
    BlendState    = 2,
    RasterState   = 3,
    DepthState    = 4,
    IndexBuffer   = 5,
    VertexBuffer  = 6,
    VertexLayout  = 7,
    PrimitiveType = 8,
    Bindings      = 9,
    RenderPass    = 10,
};

enum class StateGroupBit : uint16_t {
    VertexShader  = 1 << static_cast<uint16_t>(StateGroupIndex::VertexShader),
    PixelShader   = 1 << static_cast<uint16_t>(StateGroupIndex::PixelShader),
    BlendState    = 1 << static_cast<uint16_t>(StateGroupIndex::BlendState),
    RasterState   = 1 << static_cast<uint16_t>(StateGroupIndex::RasterState),
    DepthState    = 1 << static_cast<uint16_t>(StateGroupIndex::DepthState),
    IndexBuffer   = 1 << static_cast<uint16_t>(StateGroupIndex::IndexBuffer),
    VertexBuffer  = 1 << static_cast<uint16_t>(StateGroupIndex::VertexBuffer),
    VertexLayout  = 1 << static_cast<uint16_t>(StateGroupIndex::VertexLayout),
    PrimitiveType = 1 << static_cast<uint16_t>(StateGroupIndex::PrimitiveType),
    Bindings      = 1 << static_cast<uint16_t>(StateGroupIndex::Bindings),
    RenderPass      = 1 << static_cast<uint16_t>(StateGroupIndex::RenderPass),
};
    
    
static uint16_t AsBit(StateGroupIndex index) {
    return 1 << static_cast<uint16_t>(index);
}

struct StateGroupHeader {
    static constexpr uint32_t kMaxStateGroupFields = 16;
    
    uint16_t payloadSizeInBytes{0};
    uint16_t stateBitfield{0};
    int16_t offsets[kMaxStateGroupFields];
    uint8_t bindingCount{0};
};
        
struct StateGroup : public std::vector<uint8_t> {};
    
}
