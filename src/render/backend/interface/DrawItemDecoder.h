#pragma once

#include "DrawItem.h"
#include "DrawCall.h"
#include "Binding.h"
#include "VertexStream.h"
#include <stdint.h>
#include <vector>
#include "ResourceTypes.h"
#include <cassert>

namespace gfx {
class DrawItemDecoder {
private:
    enum class DrawItemOffset : uint8_t {
        Length = 0,
        DrawCall,
        PipelineState,
        BindingCount,
        Bindings,
        VertexStreamCount,
        VertexStreams,
        IndexBuffer,
        IndexBufferOffset,
        Name

    };

    std::uintptr_t GetOffset(const DrawItemOffset& offset) const {
        std::uintptr_t ptrOffset = 0;
        switch (offset) {
        case DrawItemOffset::Length:
            ptrOffset = 0;
            break;
        case DrawItemOffset::DrawCall:
            ptrOffset = sizeof(size_t);
            break;
        case DrawItemOffset::PipelineState:
            ptrOffset = sizeof(size_t) + sizeof(DrawCall);
            break;
        case DrawItemOffset::BindingCount:
            ptrOffset = sizeof(size_t) + sizeof(DrawCall) + sizeof(PipelineStateId);
            break;
        case DrawItemOffset::Bindings:
            ptrOffset = sizeof(size_t) + sizeof(DrawCall) + sizeof(PipelineStateId) + sizeof(size_t);
            break;
        case DrawItemOffset::VertexStreamCount:
            ptrOffset = sizeof(size_t) + sizeof(DrawCall) + sizeof(PipelineStateId) + sizeof(size_t) +
                        (bindingCount() * sizeof(Binding));
            break;
        case DrawItemOffset::VertexStreams:
            ptrOffset = sizeof(size_t) + sizeof(DrawCall) + sizeof(PipelineStateId) + sizeof(size_t) +
                        (bindingCount() * sizeof(Binding)) + sizeof(size_t);
            break;
        case DrawItemOffset::IndexBuffer:
            ptrOffset = sizeof(size_t) + sizeof(DrawCall) + sizeof(PipelineStateId) + sizeof(size_t) +
                        (bindingCount() * sizeof(Binding)) + sizeof(size_t) +
                        (vertexStreamCount() * sizeof(VertexStream));
            break;
        case DrawItemOffset::IndexBufferOffset:
            ptrOffset = sizeof(size_t) + sizeof(DrawCall) + sizeof(PipelineStateId) + sizeof(size_t) +
                        (bindingCount() * sizeof(Binding)) + sizeof(size_t) +
                        (vertexStreamCount() * sizeof(VertexStream)) + sizeof(size_t);
            break;
        }
        return reinterpret_cast<std::uintptr_t>(_item) + ptrOffset;
    }

    const DrawItem* _item{nullptr};

public:
    DrawItemDecoder(const DrawItem* item) : _item(item) {}

    size_t len() const { return static_cast<size_t>(*((size_t*)(this))); }
    const DrawCall* drawCall() const {
        return reinterpret_cast<const DrawCall*>(GetOffset(DrawItemOffset::DrawCall));
    }
    size_t bindingCount() const {
        return static_cast<size_t>(*((size_t*) (GetOffset(DrawItemOffset::BindingCount))));
    }
    const Binding* bindings() const {
        return bindingCount() ? reinterpret_cast<const Binding*>(GetOffset(DrawItemOffset::Bindings)) : nullptr;
    }
    PipelineStateId pipelineState() const {
        return static_cast<PipelineStateId>(*(PipelineStateId*) (GetOffset(DrawItemOffset::PipelineState)));
    }
    size_t vertexStreamCount() const {
        return static_cast<size_t>(*(size_t*) (GetOffset(DrawItemOffset::VertexStreamCount)));
    }
    const VertexStream* streams() const {
        return vertexStreamCount() ? reinterpret_cast<const VertexStream*>(GetOffset(DrawItemOffset::VertexStreams)) : nullptr;
    }
    BufferId indexBuffer() const {
        return static_cast<BufferId>(*(BufferId*) GetOffset(DrawItemOffset::IndexBuffer));
    }
    uint32_t indexOffset() const {
        return static_cast<uint32_t>(*(uint32_t*) GetOffset(DrawItemOffset::IndexBufferOffset));
    }
};
}
