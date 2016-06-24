#pragma once

#include "VertexStream.h"
#include "Binding.h"
#include "DrawCall.h"

namespace gfx {

struct DrawItemDesc {
    static constexpr size_t kMaxBindings      = 5;
    static constexpr size_t kMaxVertexStreams = 5;

    DrawCall drawCall;
    PipelineStateId pipelineState{0};

    size_t bindingCount{0};
    Binding bindings[kMaxBindings];

    size_t streamCount{ 0 };
    VertexStream streams[kMaxVertexStreams];
    BufferId indexBuffer{ 0 };
    uint32_t offset{ 0 };
};

static size_t GetDrawItemSize(const DrawItemDesc& desc) {
    return sizeof(size_t)                                                // total item size
           + sizeof(DrawCall) + sizeof(PipelineStateId) + sizeof(size_t) // binding count
           + sizeof(Binding) * desc.bindingCount + sizeof(size_t)        // stream count
           + sizeof(VertexStream) * desc.streamCount +
           (desc.drawCall.type == DrawCall::Type::Indexed ? sizeof(BufferId) + sizeof(uint32_t)
                                                          : 0); // indexBuffer and offset
}
}
