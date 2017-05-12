#pragma once

#include "Hash.h"
#include <stdint.h>
#include <cstddef>
#include "BlendState.h"
#include "RasterState.h"
#include "DepthState.h"
#include "PrimitiveType.h"
#include "ResourceTypes.h"

namespace gfx {
    
static const size_t kMaxColorAttachments = 4;
    
struct PipelineStateDesc {
    RenderPassId renderPass{0};
    ShaderId vertexShader{0};
    ShaderId pixelShader{0};
    VertexLayoutId vertexLayout{0};
    PrimitiveType topology{PrimitiveType::Triangles};
    size_t blendAttachmentCount{0};
    BlendState blendState[kMaxColorAttachments];
    RasterState rasterState;
    DepthState depthState;
};
}

namespace std {
template <>
struct hash<gfx::PipelineStateDesc> {
    typedef gfx::PipelineStateDesc argument_type;
    typedef std::size_t result_type;
    size_t operator()(argument_type const& s) const {
        size_t blendStateKey = 0;
        for (uint32_t i = 0; i < s.blendAttachmentCount; ++i) {
            blendStateKey = HashCombine(blendStateKey, s.blendState[i]);
        }
        
        return HashCombine(blendStateKey, s.blendAttachmentCount, s.renderPass, s.vertexShader, s.pixelShader, s.vertexLayout, s.topology, s.rasterState,
                           s.depthState);
    }
};
}
