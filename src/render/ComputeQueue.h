#pragma once

#include <algorithm>
#include <stdint.h>
#include <vector>
#include "CommandBuffer.h"
#include "ComputePassCommandBuffer.h"
#include "DispatchItem.h"
#include "DispatchCall.h"
#include "RenderDevice.h"
#include "StateGroup.h"
#include "DispatchItemDecoder.h"

using namespace gfx;

class ComputeQueue {
private:
    typedef std::pair<uint32_t, const gfx::DispatchItem*> DispatchItemPair;
    std::vector<DispatchItemPair> _items;

public:
    // temporary
    const gfx::StateGroup* defaults{ nullptr };
    const gfx::RenderPassId renderPass{ 0 };

    ComputeQueue(const gfx::StateGroup* defaults) : defaults(defaults) {}

    void AddDispatchItem(uint32_t key, const gfx::DispatchItem* item) { _items.push_back(std::make_pair(key, item)); }

    void Submit(gfx::ComputePassCommandBuffer* commandBuffer) {
        Sort();

        for (const auto& p : _items) {
            const auto* drawItem = p.second;

            PipelineStateId pipelineStateId;
            DispatchCall        dispatchCall;
            std::vector<Binding>      bindings;

            DispatchItemDecoder decoder(drawItem);

            size_t bindingCount = decoder.GetBindingCount();

            bindings.clear();
            bindings.resize(bindingCount);

            Binding*      bindingPtr = bindings.data();

            dg_assert_nm(decoder.ReadDispatchCall(&dispatchCall));
            dg_assert_nm(decoder.ReadPipelineState(&pipelineStateId));
            if (bindingCount > 0) {
                dg_assert_nm(decoder.ReadBindings(&bindingPtr));
            }

            commandBuffer->setPipelineState(pipelineStateId);

            for (const Binding& binding : bindings) {
                switch (binding.type) {
                case Binding::Type::ConstantBuffer: {
                    commandBuffer->setCBuffer(binding.resource, binding.slot);
                    break;
                }
                case Binding::Type::Texture: {
                    commandBuffer->setTexture(binding.resource, binding.slot, binding.bindingFlags);
                    break;
                }
                case Binding::Type::Buffer: {
                    commandBuffer->setBuffer(binding.resource, binding.slot, binding.bindingFlags);
                    break;
                }
                default: {
                    dg_assert_fail_nm();
                }
                }
            }
            switch (dispatchCall.type) {
            case DispatchCall::Type::Direct: {
                commandBuffer->dispatch(dispatchCall.groupX, dispatchCall.groupY, dispatchCall.groupZ);
                break;
            }
            default:
                dg_assert_fail_nm();
            }
        }
    }

private:
    void Sort() {
        std::sort(begin(_items), end(_items), [](const auto& p1, const auto& p2) { return p1.first < p2.first; });
    }
};
