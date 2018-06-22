//
//  RenderPassCommandBuffer.h
//  planet
//
//  Created by Eugene Sturm on 6/22/18.
//

#pragma once

#include "ResourceTypes.h"
#include "ShaderStageFlags.h"

namespace gfx
{
    class RenderPassCommandBuffer
    {
    public:
        virtual void setPipelineState(PipelineStateId pipelineState) = 0;
        virtual void setVertexBuffer(BufferId vertexBuffer) = 0;
        virtual void setShaderBuffer(BufferId buffer, uint8_t index, ShaderStageFlags stages) = 0;
        virtual void setShaderTexture(TextureId texture, uint8_t index, ShaderStageFlags stages) = 0;
        virtual void drawIndexed(BufferId indexBufferId, uint32_t indexCount, uint32_t indexOffset, uint32_t baseVertexOffset) = 0;
    };
}
