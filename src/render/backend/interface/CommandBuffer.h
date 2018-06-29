#pragma once

#include "ResourceTypes.h"
#include "Framebuffer.h"

namespace gfx
{
    class RenderPassCommandBuffer;
    
    class CommandBuffer {
    public:
        virtual RenderPassCommandBuffer* beginRenderPass(RenderPassId passId, const FrameBuffer& frameBuffer) = 0;
        virtual void endRenderPass(RenderPassCommandBuffer* commandBuffer) = 0;
    };
}
