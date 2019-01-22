#pragma once

#include "ResourceTypes.h"
#include "Framebuffer.h"

namespace gfx
{
    class RenderPassCommandBuffer;
    class ComputePassCommandBuffer;
    
    class CommandBuffer {
    public:
        virtual RenderPassCommandBuffer* beginRenderPass(RenderPassId passId, const FrameBuffer& frameBuffer, const std::string& name) = 0;
        virtual void endRenderPass(RenderPassCommandBuffer* commandBuffer) = 0;

        virtual ComputePassCommandBuffer* beginComputePass(const std::string& name) = 0;
        virtual void endComputePass(ComputePassCommandBuffer* commandBuffer) = 0;
    };
}
