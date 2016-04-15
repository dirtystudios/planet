#pragma once

#include <stdint.h>
#include <cstddef>
#include "BlendState.h"
#include "RasterState.h"
#include "DepthState.h"
#include "PrimitiveType.h"
#include "ResourceTypes.h"

namespace graphics {
struct PipelineStateDesc {
    ShaderId vertexShader{0};
    ShaderId pixelShader{0};
    VertexLayoutId vertexLayout{0};
    PrimitiveType topology;
    BlendState blendState;
    RasterState rasterState;
    DepthState depthState;
};
}