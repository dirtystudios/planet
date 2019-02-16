#pragma once

#include <stdint.h>
#include "StateGroup.h"
#include "Binding.h"
#include "RasterState.h"
#include "BlendState.h"
#include "DepthState.h"
#include "Bytebuffer.h"
#include "PrimitiveType.h"
#include "ShaderStageFlags.h"

namespace gfx {
class StateGroupEncoder {
private:
    static constexpr size_t kStagingBufferSize = 256;

    StateGroupHeader     _currentHeader;
    ByteBuffer           _groupStagingBuffer;
    std::vector<Binding> _bindingStagingBuffer;

public:
    static const StateGroup* Merge(const StateGroup* const* stateGroups, uint32_t count);
    static const StateGroup* Merge(const std::vector<const StateGroup*>& stateGroups);

    void Begin(const StateGroup* inherit = nullptr);
    const StateGroup* End();

    void SetRenderPass(RenderPassId ps);
    void SetVertexBuffer(BufferId vb);
    void SetIndexBuffer(BufferId ib);
    void SetVertexLayout(VertexLayoutId vl);
    void SetVertexShader(ShaderId vs);
    void SetPixelShader(ShaderId ps);
    void SetComputeShader(ShaderId cs);
    void SetPrimitiveType(PrimitiveType pt);
    void SetBlendState(const BlendState& bs);
    void SetRasterState(const RasterState& rs);
    void SetDepthState(const DepthState& ds);
    void BindTexture(uint32_t slot, TextureId tex, ShaderStageFlags flags = ShaderStageFlags::AllStages, ShaderBindingFlags bindingFlags = ShaderBindingFlags::SampleRead);
    void BindBuffer(uint32_t slot, BufferId tex, ShaderStageFlags flags = ShaderStageFlags::AllStages, ShaderBindingFlags bindingFlags = ShaderBindingFlags::ReadOnly);
    void BindConstantBuffer(uint32_t slot, BufferId cb, ShaderStageFlags flags = ShaderStageFlags::AllStages);
    void BindResource(uint32_t slot, Binding::Type type, ResourceId resource,
                      ShaderStageFlags flags, ShaderBindingFlags bindingflags);
    void BindResource(const Binding& binding);

private:
    void WriteState(StateGroupBit bit, StateGroupIndex idx, const void* data, size_t len);
    bool HasBinding(const Binding& binding);
};
}
