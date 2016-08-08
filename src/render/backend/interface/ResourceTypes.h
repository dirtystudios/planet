#pragma once

#include <cstdlib>

namespace gfx {
using TextureId       = size_t;
using ShaderParamId   = size_t;
using PipelineStateId = size_t;
using BufferId        = size_t;
using ShaderId        = size_t;
using VertexLayoutId  = size_t;
using ResourceId      = size_t;

enum class ResourceType : uint8_t {
    Texture = 0,
    ShaderParam,
    PipelineState,
    Buffer,
    Shader,
    VertexLayout,
};
}
