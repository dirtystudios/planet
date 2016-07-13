#pragma once

#include <stdint.h>
#include "StateGroup.h"
#include "Binding.h"
#include "RasterState.h"
#include "BlendState.h"
#include "DepthState.h"

namespace gfx {
class StateGroupDecoder {
private:
    const StateGroup* _stateGroup{nullptr};
    StateGroupHeader _header;
public:
    StateGroupDecoder(const StateGroup* sg) : _stateGroup(sg) {
        assert(_stateGroup);
        memcpy(&_header, _stateGroup->data(), sizeof(StateGroupHeader));
    };
    ~StateGroupDecoder() { _stateGroup = nullptr; };
    
    const StateGroupHeader& GetHeader() { return _header; }
    size_t GetSize() { return _stateGroup->size(); }
    uint8_t GetBindingCount() { return _header.bindingCount; }
    uint16_t GetStateBitfield() { return _header.stateBitfield; }
    int16_t GetOffset(StateGroupIndex index) { return _header.offsets[static_cast<size_t>(index)]; }
    bool HasState(StateGroupIndex index) { return HasState(1 << static_cast<uint16_t>(index)); }
    bool HasState(StateGroupBit bit) { return HasState(static_cast<uint16_t>(bit)); }
    bool HasState(uint16_t bit) { return _header.stateBitfield & bit; }
    bool ReadVertexShader(ShaderId* shaderId) {
        return ReadState(StateGroupIndex::VertexShader, shaderId);
    }
    bool ReadPixelShader(ShaderId* shaderId) {
        return ReadState(StateGroupIndex::PixelShader, shaderId);
    }
    bool ReadVertexLayout(VertexLayoutId* vertexLayoutId) {
        return ReadState(StateGroupIndex::VertexLayout, vertexLayoutId);
    }
    bool ReadBlendState(BlendState* blendState) {
        return ReadState(StateGroupIndex::BlendState, blendState);
    }
    bool ReadRasterState(RasterState* rasterState) {
        return ReadState(StateGroupIndex::RasterState, rasterState);
    }
    bool ReadDepthState(DepthState* depthState) {
        return ReadState(StateGroupIndex::DepthState, depthState);
    }
    bool ReadIndexBuffer(BufferId* bufferId) {
        return ReadState(StateGroupIndex::IndexBuffer, bufferId);
    }
    bool ReadVertexBuffer(BufferId* bufferId) {
        return ReadState(StateGroupIndex::VertexBuffer, bufferId);
    }
    bool ReadBindings(Binding** bindingOut) {
        assert(bindingOut);
        return ReadState(StateGroupIndex::Bindings, *bindingOut, _header.bindingCount);
    }
    const uint8_t* GetPtr(StateGroupIndex index) {
        int16_t offset = GetOffset(index);
        return offset == -1 ? nullptr : &_stateGroup->data()[offset];
    }
private:
    template <class T> bool ReadState(StateGroupIndex index, T* stateOut, uint32_t count = 1) {
        assert(stateOut);
        if (!HasState(static_cast<StateGroupBit>(1 << (int)index))) {
            return false;
        } else {
            memcpy(stateOut, GetPtr(index), sizeof(T) * count);
            return true;
        }
    }
    
    
    
    
};
}
