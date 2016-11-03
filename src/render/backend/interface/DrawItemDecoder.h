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
    const DrawItem* _drawItem{nullptr};
    uint8_t         _streamCount{0};
    uint8_t         _bindingCount{0};

public:
    DrawItemDecoder(const DrawItem* item) : _drawItem(item) {
        dg_assert_nm(item != nullptr);
        dg_assert_nm(item->size() != 0 && item->size() != 1024); // shoudnt be bigger than 1k. even thats a massive stretch
        ReadState(DrawItemField::StreamCount, &_streamCount);
        ReadState(DrawItemField::BindingCount, &_bindingCount);
    }

    size_t GetStreamCount() { return _streamCount; }

    size_t GetBindingCount() { return _bindingCount; }

    bool ReadDrawCall(DrawCall* drawCall) {
        return ReadState(DrawItemField::DrawCall, drawCall);
    }
    
    bool ReadPipelineState(PipelineStateId* pipelineState) {
        return ReadState(DrawItemField::PipelineState, pipelineState);
    }
    
    bool ReadIndexBuffer(BufferId* indexBuffer) {
        return ReadState(DrawItemField::IndexBuffer, indexBuffer);
    }

    bool ReadVertexStreams(VertexStream** streams) {
        return ReadState(DrawItemField::VertexStreams, *streams, _streamCount);
    }

    bool ReadBindings(Binding** bindings) { return ReadState(DrawItemField::Bindings, *bindings, _bindingCount); }

private:
    template <class T> bool ReadState(DrawItemField field, T* stateOut, uint32_t count = 1) {
        assert(stateOut);
        size_t bytesToRead = sizeof(T) * count;
        size_t offset = GetOffset(field);
        assert(offset != -1 && _drawItem->size() >= offset + bytesToRead);
        memcpy(stateOut, _drawItem->data() + offset, bytesToRead);
        return true;
    }

    size_t GetOffset(DrawItemField field) {
        switch (field) {
        case DrawItemField::StreamCount:
            return 0;
            case DrawItemField::BindingCount: return sizeof(uint8_t);
            case DrawItemField::DrawCall: return GetOffset(DrawItemField::BindingCount) + sizeof(uint8_t);
            case DrawItemField::IndexBuffer:  return GetOffset(DrawItemField::DrawCall) + sizeof(DrawCall);
            case DrawItemField::PipelineState: return GetOffset(DrawItemField::IndexBuffer) + sizeof(BufferId);
            case DrawItemField::VertexStreams: return GetOffset(DrawItemField::PipelineState) + sizeof(PipelineStateId);
            case DrawItemField::Bindings: return GetOffset(DrawItemField::VertexStreams) + sizeof(VertexStream) * _streamCount;
            default: assert(false);
        }
        return -1;
    }

};

}
