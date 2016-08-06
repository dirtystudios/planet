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
struct PipelineStateDesc {
    ShaderId vertexShader{0};
    ShaderId pixelShader{0};
    VertexLayoutId vertexLayout{0};
    PrimitiveType topology{PrimitiveType::Triangles};
    BlendState blendState;
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
        return HashCombine(s.vertexShader, s.pixelShader, s.vertexLayout, s.topology, s.blendState, s.rasterState,
                           s.depthState);
    }
};
}
