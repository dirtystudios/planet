#pragma once

#include "ResourceTypes.h"
#include "ShaderStageFlags.h"

namespace gfx
{
    class ComputePassCommandBuffer
    {
    public:
        virtual void setPipelineState(PipelineStateId pipelineState) = 0;
        virtual void setCBuffer(BufferId buffer, uint8_t index) = 0;
        virtual void setBuffer(BufferId buffer, uint8_t index) = 0;
        virtual void setTexture(TextureId texture, uint8_t index) = 0;
        virtual void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;
    };
}
