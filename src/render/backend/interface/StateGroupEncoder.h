#pragma once

#include <stdint.h>
#include "StateGroup.h"
#include "Binding.h"
#include "RasterState.h"
#include "BlendState.h"
#include "DepthState.h"
#include "Bytebuffer.h"

namespace gfx {
class StateGroupEncoder {
private:
    static constexpr size_t kStagingBufferSize = 256;
    
    StateGroupHeader _currentHeader;
    ByteBuffer _groupStagingBuffer;    
    std::vector<Binding> _bindingStagingBuffer;    
public:
    static const StateGroup* Merge(const StateGroup* const* stateGroups, uint32_t count);

    void Begin(const StateGroup* inherit = nullptr);
    const StateGroup* End();

    void SetVertexBuffer(BufferId vb);
    void SetIndexBuffer(BufferId ib);
    void SetVertexLayout(VertexLayoutId vl);
    void SetVertexShader(ShaderId vs);
    void SetPixelShader(ShaderId ps);
    void SetBlendState(const BlendState& bs);
    void SetRasterState(const RasterState& rs);
    void SetDepthState(const DepthState& ds);
    void BindTexture(uint32_t slot, TextureId tex);
    void BindConstantBuffer(uint32_t slot, BufferId cb);
    void BindResource(uint32_t slot, Binding::Type type, ResourceId resource);
    void BindResource(const Binding& binding);
private:    
    
    void WriteState(StateGroupBit bit, StateGroupIndex idx, const void* data, size_t len);
    bool HasBinding(const Binding& binding);
};
}
