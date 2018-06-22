#pragma once

#include <algorithm>
#include <stdint.h>
#include <vector>
#include "CommandBuffer.h"
#include "RenderPassCommandBuffer.h"
#include "DrawItem.h"
#include "RenderDevice.h"
#include "StateGroup.h"
#include "DrawItemDecoder.h"
#include "VertexStream.h"

using namespace gfx;

class RenderQueue {
private:
    typedef std::pair<uint32_t, const gfx::DrawItem*> DrawItemPair;
    std::vector<DrawItemPair> _items;

public:
    // temporary
    const gfx::StateGroup* defaults{nullptr};
    const gfx::RenderPassId renderPass { 0 };

    RenderQueue(gfx::RenderPassId renderPass, const gfx::StateGroup* defaults) : renderPass(renderPass), defaults(defaults) {}

    void AddDrawItem(uint32_t key, const gfx::DrawItem* item) { _items.push_back(std::make_pair(key, item)); }

    void Submit(gfx::RenderPassCommandBuffer* commandBuffer) {
        Sort();
        
        for (const DrawItemPair& p : _items) {
            const gfx::DrawItem* drawItem = p.second;
            
            PipelineStateId pipelineStateId;
            DrawCall        drawCall;
            BufferId        indexBufferId;
            std::vector<VertexStream> streams;
            std::vector<Binding>      bindings;
            
            DrawItemDecoder decoder(drawItem);
            
            size_t streamCount = decoder.GetStreamCount();
            dg_assert(streamCount == 1, "> 1 stream count not supported");
            size_t bindingCount = decoder.GetBindingCount();
            
            streams.clear();
            bindings.clear();
            streams.resize(streamCount);
            bindings.resize(bindingCount);
            
            VertexStream* streamPtr  = streams.data();
            Binding*      bindingPtr = bindings.data();
            
            dg_assert_nm(decoder.ReadDrawCall(&drawCall));
            dg_assert_nm(decoder.ReadPipelineState(&pipelineStateId));
            dg_assert_nm(decoder.ReadIndexBuffer(&indexBufferId));
            dg_assert_nm(decoder.ReadVertexStreams(&streamPtr));
            if (bindingCount > 0) {
                dg_assert_nm(decoder.ReadBindings(&bindingPtr));
            }
            
            commandBuffer->setPipelineState(pipelineStateId);
            commandBuffer->setVertexBuffer(streamPtr[0].vertexBuffer);
            
            for (const Binding& binding : bindings) {
                switch (binding.type) {
                    case Binding::Type::ConstantBuffer: {
                        commandBuffer->setShaderBuffer(binding.resource, binding.slot, binding.stageFlags);
                        break;
                    }
                    case Binding::Type::Texture: {
                        commandBuffer->setShaderTexture(binding.resource, binding.slot, binding.stageFlags);
                        break;
                    }
                    default: {
                        dg_assert_fail_nm();
                    }
                }
            }
            switch (drawCall.type) {
                case DrawCall::Type::Arrays: {
                    // TODO
//
//                    [encoder drawPrimitives:primitiveType
//                                vertexStart:drawCall.startOffset
//                                vertexCount:drawCall.primitiveCount];
                    break;
                }
                case DrawCall::Type::Indexed: {
                    commandBuffer->drawIndexed(indexBufferId, drawCall.primitiveCount, drawCall.startOffset, drawCall.baseVertexOffset);
                    break;
                }
                default:
                    dg_assert_fail_nm();
            }
        }
    }

private:
    void Sort() {
        std::sort(begin(_items), end(_items), [](const DrawItemPair& p1, const DrawItemPair& p2) { return p1.first < p2.first; });
    }
};
