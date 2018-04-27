#pragma once

#include <cstdlib>
#include <cstdint>

namespace gfx {
    using TextureId       = size_t;
    using PipelineStateId = size_t;
    using BufferId        = size_t;
    using ShaderId        = size_t;
    using VertexLayoutId  = size_t;
    using ResourceId      = size_t;
    using RenderPassId    = size_t;
    
    enum class ResourceType : uint8_t {
        Texture = 0,
        ShaderParam,
        PipelineState,
        Buffer,
        Shader,
        RenderPass,
        VertexLayout,
    };
}
