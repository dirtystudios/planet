#include "DrawItemEncoder.h"
#include "StateGroupEncoder.h"
#include "StateGroupDecoder.h"
#include "PipelineStateDesc.h"
#include "RenderDevice.h"
#include "VertexStream.h"
#include <cassert>

namespace gfx {

PipelineStateId GetPipelineState(gfx::RenderDevice* device, StateGroupDecoder& decoder) {
    gfx::PipelineStateDesc desc;

    dg_assert_nm(decoder.ReadRenderPass(&desc.renderPass));
    dg_assert_nm(decoder.ReadPixelShader(&desc.pixelShader));
    dg_assert_nm(decoder.ReadVertexShader(&desc.vertexShader));
    dg_assert_nm(decoder.ReadVertexLayout(&desc.vertexLayout));

    // these can be defaulted so no assert
    decoder.ReadBlendState(&desc.blendState);
    decoder.ReadRasterState(&desc.rasterState);
    decoder.ReadDepthState(&desc.depthState);
    decoder.ReadPrimitiveType(&desc.topology);

    PipelineStateId psId = device->CreatePipelineState(desc);
    assert(psId);

    // TODO: This is a huge probelm, just creating pipeline states, and no way of cleaning them up
    return psId;
}

const DrawItem* DrawItemEncoder::Encode(RenderDevice* device, const DrawCall& drawCall, const std::vector<const StateGroup*>& stateGroups) {
    return Encode(device, drawCall, stateGroups.data(), stateGroups.size());
}

const DrawItem* DrawItemEncoder::Encode(gfx::RenderDevice* device, const DrawCall& drawCall, const StateGroup* const* stateGroups, uint32_t count) {
    const StateGroup* stateGroup    = nullptr;
    bool              shouldCleanUp = false;

    if (count > 1) {
        // merge into temporary buffer
        shouldCleanUp = true;
        stateGroup    = StateGroupEncoder::Merge(stateGroups, count);
    } else {
        stateGroup = stateGroups[0];
    }

    StateGroupDecoder decoder(stateGroup);

    PipelineStateId pipelineState = GetPipelineState(device, decoder);
    uint8_t bindingCount          = decoder.GetBindingCount();
    BufferId indexBuffer          = 0;
    uint8_t streamCount           = 1;
    VertexStream stream;
    dg_assert_nm(decoder.ReadVertexBuffer(&stream.vertexBuffer));
    stream.offset = 0; // unused until we try to Read more complicated
    stream.stride = 0; // unused until we try to Read more complicated

    if (drawCall.type == DrawCall::Type::Indexed) {
        dg_assert_nm(decoder.ReadIndexBuffer(&indexBuffer));
    }

    size_t drawItemSize = sizeof(uint8_t) * 2 + bindingCount * sizeof(Binding) + sizeof(DrawCall) +
                          sizeof(PipelineStateId) + sizeof(BufferId) + sizeof(VertexStream) * streamCount;
    DrawItem* di = new DrawItem();
    di->resize(drawItemSize);

    ByteBuffer writer(di->data(), di->size());

    writer.Write(&streamCount, sizeof(uint8_t));
    writer.Write(&bindingCount, sizeof(uint8_t));
    writer.Write(&drawCall, sizeof(DrawCall));
    writer.Write(&indexBuffer, sizeof(BufferId)); // 0 if no indexbuffer
    writer.Write(&pipelineState, sizeof(PipelineStateId));
    writer.Write(&stream, sizeof(VertexStream));
    if (decoder.HasState(StateGroupIndex::Bindings)) {
        int16_t bindingOffset = decoder.GetOffset(StateGroupIndex::Bindings);
        dg_assert_nm(bindingOffset != -1);
        writer.Write(stateGroup->data() + bindingOffset, stateGroup->size() - bindingOffset);
    }

    if (shouldCleanUp) {
        delete stateGroup;
    }
    
    return di;
}
    
    
}
