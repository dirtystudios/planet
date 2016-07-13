#pragma once

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
template <> struct hash<gfx::PipelineStateDesc> {
    typedef gfx::PipelineStateDesc argument_type;
    typedef std::size_t result_type;
    result_type operator()(argument_type const& s) const {
        result_type key = 0;
        HashCombine(key, s.vertexShader);
        HashCombine(key, s.pixelShader);
        HashCombine(key, s.vertexLayout);
        HashCombine(key, s.topology);
        HashCombine(key, s.blendState);
        HashCombine(key, s.rasterState);
        HashCombine(key, s.depthState);
        return key;
    }
};
}
