#include "StateGroupEncoder.h"
#include "StateGroupDecoder.h"

static constexpr size_t kMaxStateGroupPayloadSize = 512 - sizeof(gfx::StateGroupHeader);

namespace gfx {
size_t GetStateGroupFieldSize(const StateGroupHeader& header, StateGroupIndex index) {
    switch(index) {
        case StateGroupIndex::VertexBuffer:
        case StateGroupIndex::IndexBuffer:
        case StateGroupIndex::VertexShader:
        case StateGroupIndex::PixelShader:
        case StateGroupIndex::VertexLayout:
            return sizeof(ResourceId);
        case StateGroupIndex::BlendState: return sizeof(BlendState);
        case StateGroupIndex::RasterState: return sizeof(RasterState);
        case StateGroupIndex::DepthState: return sizeof(DepthState);
        case StateGroupIndex::Bindings: return header.bindingCount * sizeof(Binding);
        default: assert(false); return 0;
    }
}
    
void StateGroupEncoder::Begin(const StateGroup* inherit) {
    memset(&_currentHeader, 0, sizeof(StateGroupHeader));
    memset(&_currentHeader.offsets, -1, sizeof(int16_t) * StateGroupHeader::kMaxStateGroupFields);
    _groupStagingBuffer.Reset();
    _bindingStagingBuffer.clear();

    if (inherit) {
        memcpy(&_currentHeader, inherit->data(), sizeof(StateGroupHeader));
        
        for(uint32_t idx = 0; idx < 16; ++idx) {
            if(_currentHeader.offsets[idx] != -1) {
                _currentHeader.offsets[idx] -= sizeof(StateGroupHeader);
            }
        }
        
        StateGroupDecoder decoder(inherit);
        size_t bytesToCopy = inherit->size() - sizeof(StateGroupHeader);
        
        if(_currentHeader.bindingCount > 0) {
            _bindingStagingBuffer.resize(_currentHeader.bindingCount);
            Binding* b = _bindingStagingBuffer.data();
            decoder.ReadBindings(&b);
            bytesToCopy = _currentHeader.offsets[static_cast<uint16_t>(StateGroupIndex::Bindings)];
            assert(bytesToCopy != -1);
            _currentHeader.stateBitfield &= ~(AsBit(StateGroupIndex::Bindings));
            _currentHeader.payloadSizeInBytes = bytesToCopy;
        }
        _groupStagingBuffer.Write(inherit->data() + sizeof(StateGroupHeader), bytesToCopy);
    }
}
    
const StateGroup* StateGroupEncoder::Merge(const StateGroup* const* stateGroups, uint32_t count) {
    assert(stateGroups && stateGroups[0]);
    if(count == 1) {
        return stateGroups[0];
    }
    
    StateGroupEncoder encoder;
    const StateGroup* primarySG = stateGroups[0];
    StateGroupDecoder primaryDecoder(primarySG);
    
    encoder.Begin(primarySG);
    
    uint16_t openStates = ~(primaryDecoder.GetStateBitfield());
    openStates |= static_cast<uint16_t>(StateGroupBit::Bindings); // bindings is always open
    
    for (uint32_t idx = 1; idx < count; ++idx) {
        const StateGroup* candidateSG = stateGroups[idx];
        StateGroupDecoder candidateDecoder(candidateSG);
        const StateGroupHeader& header = candidateDecoder.GetHeader();

        if (!(openStates & candidateDecoder.GetStateBitfield())) {
            continue;
        }

        for (uint32_t shift = 0; shift < 16; ++shift) {
            uint16_t stateBit = 1 << shift;
            bool isOpen = ((openStates & stateBit) > 0);
            if (!isOpen || !candidateDecoder.HasState(stateBit)) {
                continue;
            }

            openStates &= ~(stateBit);

            switch (static_cast<StateGroupBit>(stateBit)) {
#define LZY(x)                                                                                                         \
    case StateGroupBit::x: {                                                                                           \
        encoder.WriteState(StateGroupBit::x, StateGroupIndex::x, &candidateSG->data()[header.offsets[static_cast<uint16_t>(StateGroupIndex::x)]], GetStateGroupFieldSize(header, StateGroupIndex::x));\
        break;                                                                                                         \
    }
                LZY(BlendState)
                LZY(RasterState)
                LZY(VertexBuffer)
                LZY(VertexLayout)
                LZY(IndexBuffer)
                LZY(VertexShader)
                LZY(PixelShader)
                LZY(DepthState)
                case StateGroupBit::Bindings: {

                    std::vector<Binding> bindings(header.bindingCount);
                    Binding* ptr = bindings.data();
                    candidateDecoder.ReadBindings(&ptr);

                    for (const Binding& binding : bindings) {
                        encoder.BindResource(binding.slot, binding.type, binding.resource, binding.stageFlags);
                    }
                    break;
                }
                default:
                    assert(false);
            }
            // always keep this one open
            openStates |= AsBit(StateGroupIndex::Bindings);
        }
    }
    return encoder.End();
}

void StateGroupEncoder::SetVertexBuffer(BufferId vb) {
    WriteState(StateGroupBit::VertexBuffer, StateGroupIndex::VertexBuffer, &vb, sizeof(BufferId));
}
void StateGroupEncoder::SetIndexBuffer(BufferId ib) {
    WriteState(StateGroupBit::IndexBuffer, StateGroupIndex::IndexBuffer, &ib, sizeof(BufferId));
}
void StateGroupEncoder::SetVertexLayout(VertexLayoutId vl) {
    WriteState(StateGroupBit::VertexLayout, StateGroupIndex::VertexLayout, &vl, sizeof(VertexLayoutId));
}
void StateGroupEncoder::SetVertexShader(ShaderId vs) {
    WriteState(StateGroupBit::VertexShader, StateGroupIndex::VertexShader, &vs, sizeof(ShaderId));
}
void StateGroupEncoder::SetPixelShader(ShaderId ps) {
    WriteState(StateGroupBit::PixelShader, StateGroupIndex::PixelShader, &ps, sizeof(ShaderId));
}
void StateGroupEncoder::SetBlendState(const BlendState& bs) {
    WriteState(StateGroupBit::BlendState, StateGroupIndex::BlendState, &bs, sizeof(BlendState));
}
void StateGroupEncoder::SetRasterState(const RasterState& rs) {
    WriteState(StateGroupBit::RasterState, StateGroupIndex::RasterState, &rs, sizeof(RasterState));
}
void StateGroupEncoder::SetDepthState(const DepthState& ds) {
    WriteState(StateGroupBit::DepthState, StateGroupIndex::DepthState, &ds, sizeof(DepthState));
}
void StateGroupEncoder::BindTexture(uint32_t slot, TextureId tex, ShaderStageFlags flags) {
    BindResource(slot, Binding::Type::Texture, tex, flags);
}
void StateGroupEncoder::BindConstantBuffer(uint32_t slot, BufferId cb, ShaderStageFlags flags) {
    BindResource(slot, Binding::Type::ConstantBuffer, cb, flags);
}

const StateGroup* StateGroupEncoder::End() {
    StateGroup* sg = new StateGroup();

    if (_bindingStagingBuffer.size() > 0) {
        // TODO: pack the bindings?
        WriteState(StateGroupBit::Bindings, StateGroupIndex::Bindings, _bindingStagingBuffer.data(),
                   _bindingStagingBuffer.size() * sizeof(Binding));
    }

    for(uint32_t idx = 0; idx < 16; ++idx) {
        if(_currentHeader.offsets[idx] != -1) {
            _currentHeader.offsets[idx] += sizeof(StateGroupHeader);
        }
    }
    
    assert(_groupStagingBuffer.WritePos() <= kMaxStateGroupPayloadSize);
    
    _currentHeader.payloadSizeInBytes = _groupStagingBuffer.WritePos();
    size_t stateGroupSize = sizeof(StateGroupHeader) + _currentHeader.payloadSizeInBytes;
    sg->resize(stateGroupSize);
    size_t offset = 0;
    memcpy(sg->data() + offset, &_currentHeader, sizeof(StateGroupHeader));
    offset += sizeof(StateGroupHeader);
    memcpy(sg->data() + offset, _groupStagingBuffer.GetDataPtr(), _groupStagingBuffer.WritePos());

    return sg;
}

void StateGroupEncoder::BindResource(uint32_t slot, Binding::Type type, ResourceId resource, ShaderStageFlags flags) {
    Binding binding;
    binding.type       = type;
    binding.slot       = slot;
    binding.resource   = resource;
    binding.stageFlags = flags;

    BindResource(binding);
}

void StateGroupEncoder::BindResource(const Binding& binding) {
    if (!HasBinding(binding)) {
        ++_currentHeader.bindingCount;
        _bindingStagingBuffer.push_back(binding);
    }
}
void StateGroupEncoder::WriteState(StateGroupBit bit, StateGroupIndex idx, const void* data, size_t len) {
    assert(data);
    int16_t* offset = &_currentHeader.offsets[static_cast<int16_t>(idx)];

    if (!(_currentHeader.stateBitfield & static_cast<uint16_t>(bit))) {
        _currentHeader.stateBitfield |= static_cast<uint16_t>(bit);
        *offset = _groupStagingBuffer.WritePos();
    }
    
    _groupStagingBuffer.Write(data, len);
}

bool StateGroupEncoder::HasBinding(const Binding& binding) {
    if (_currentHeader.bindingCount == 0) {
        return false;
    }

    for(const Binding& activeBinding : _bindingStagingBuffer) {
        if (memcmp(&binding, &activeBinding, sizeof(Binding)) == 0) {
            return true;
        }
    }
    return false;
}
} // namespace
