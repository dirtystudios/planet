#include "DispatchItemEncoder.h"
#include "StateGroupEncoder.h"
#include "StateGroupDecoder.h"
#include "PipelineStateDesc.h"
#include "RenderDevice.h"
#include <cassert>

namespace gfx {

    static PipelineStateId GetPipelineState(gfx::RenderDevice* device, StateGroupDecoder& decoder) {
        gfx::PipelineStateDesc desc;

        dg_assert_nm(decoder.ReadRenderPass(&desc.renderPass));
        dg_assert_nm(decoder.ReadComputeShader(&desc.computeShader));

        PipelineStateId psId = device->CreatePipelineState(desc);
        assert(psId);

        // TODO: This is a huge probelm, just creating pipeline states, and no way of cleaning them up
        return psId;
    }

    const DispatchItem* DispatchItemEncoder::Encode(RenderDevice* device, const DispatchCall& drawCall, const std::vector<const StateGroup*>& stateGroups) {
        return Encode(device, drawCall, stateGroups.data(), stateGroups.size());
    }

    const DispatchItem* DispatchItemEncoder::Encode(gfx::RenderDevice* device, const DispatchCall& drawCall, const StateGroup* const* stateGroups, uint32_t count) {
        const StateGroup* stateGroup = nullptr;
        bool              shouldCleanUp = false;

        if (count > 1) {
            // merge into temporary buffer
            shouldCleanUp = true;
            stateGroup = StateGroupEncoder::Merge(stateGroups, count);
        }
        else {
            stateGroup = stateGroups[0];
        }

        StateGroupDecoder decoder(stateGroup);

        PipelineStateId pipelineState = GetPipelineState(device, decoder);
        uint8_t bindingCount = decoder.GetBindingCount();

        if (drawCall.type == DispatchCall::Type::Indirect) {
            dg_assert_nm(false);
        }

        size_t drawItemSize = sizeof(uint8_t) + bindingCount * sizeof(Binding) + sizeof(DispatchCall) +
            sizeof(PipelineStateId);
        DispatchItem* di = new DispatchItem();
        di->resize(drawItemSize);

        ByteBuffer writer(di->data(), di->size());
        writer.Write(&bindingCount, sizeof(uint8_t));
        writer.Write(&drawCall, sizeof(DispatchCall));
        writer.Write(&pipelineState, sizeof(PipelineStateId));
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
