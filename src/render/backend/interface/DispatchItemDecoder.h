#pragma once

#include "DispatchItem.h"
#include "DispatchCall.h"
#include "Binding.h"
#include <stdint.h>
#include <vector>
#include "ResourceTypes.h"
#include <cassert>

namespace gfx {

class DispatchItemDecoder {
private:
    const DispatchItem* _dispatchItem{nullptr};
    uint8_t         _bindingCount{0};

public:
    DispatchItemDecoder(const DispatchItem* item) : _dispatchItem(item) {
        dg_assert_nm(item != nullptr);
        dg_assert_nm(item->size() != 0 && item->size() != 1024); // shoudnt be bigger than 1k. even thats a massive stretch
        ReadState(DispatchItemField::BindingCount, &_bindingCount);
    }

    size_t GetBindingCount() { return _bindingCount; }

    bool ReadDispatchCall(DispatchCall* dcall) {
        return ReadState(DispatchItemField::DispatchCall, dcall);
    }
    
    bool ReadPipelineState(PipelineStateId* pipelineState) {
        return ReadState(DispatchItemField::PipelineState, pipelineState);
    }

    bool ReadBindings(Binding** bindings) { return ReadState(DispatchItemField::Bindings, *bindings, _bindingCount); }

private:
    template <class T> bool ReadState(DispatchItemField field, T* stateOut, uint32_t count = 1) {
        assert(stateOut);
        size_t bytesToRead = sizeof(T) * count;
        size_t offset = GetOffset(field);
        assert(offset != -1 && _dispatchItem->size() >= offset + bytesToRead);
        memcpy(stateOut, _dispatchItem->data() + offset, bytesToRead);
        return true;
    }

    size_t GetOffset(DispatchItemField field) {
        switch (field) {
        case DispatchItemField::BindingCount:
            return 0;
            case DispatchItemField::DispatchCall: return sizeof(uint8_t);
            case DispatchItemField::PipelineState: return GetOffset(DispatchItemField::DispatchCall) + sizeof(DispatchCall);
            case DispatchItemField::Bindings: return GetOffset(DispatchItemField::PipelineState) + sizeof(PipelineStateId);
            default: assert(false);
        }
        return -1;
    }

};

}
