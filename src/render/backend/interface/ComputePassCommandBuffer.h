#pragma once

#include "ResourceTypes.h"
#include "ShaderStageFlags.h"

namespace gfx
{
    class ComputePassCommandBuffer
    {
    public:
        virtual void setPipelineState(PipelineStateId pipelineState) = 0;
        virtual void setShaderBuffer(BufferId buffer, uint8_t index, ShaderStageFlags stages) = 0;
        virtual void setShaderTexture(TextureId texture, uint8_t index, ShaderStageFlags stages) = 0;
        virtual void doTheComputeStuff(void* unk) = 0;
    };
}
