#pragma once

#include "ResourceTypes.h"
#include "Framebuffer.h"
#include <string>

namespace gfx
{
    class RenderPassCommandBuffer;
    
    class CommandBuffer {
    public:
        virtual RenderPassCommandBuffer* beginRenderPass(RenderPassId passId, const FrameBuffer& frameBuffer, const std::string& name) = 0;
        virtual void endRenderPass(RenderPassCommandBuffer* commandBuffer) = 0;
    };
}
