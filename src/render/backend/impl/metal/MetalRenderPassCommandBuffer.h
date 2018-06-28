//
//  MetalRenderPassCommandBuffer.h
//  planet
//
//  Created by Eugene Sturm on 6/28/18.
//

#pragma once

#import <Metal/Metal.h>
#include "RenderPassCommandBuffer.h"
#include "ResourceTypes.h"
#include "ShaderStageFlags.h"

namespace gfx
{
    class ResourceManager;
    class MetalPipelineState;
    
    class MetalRenderPassCommandBuffer : public RenderPassCommandBuffer
    {
    private:
        id<MTLRenderCommandEncoder> _encoder;
        ResourceManager* _resourceManager { nullptr };
        MetalPipelineState* _currentPipelineState { nullptr };
    public:
        MetalRenderPassCommandBuffer(id<MTLRenderCommandEncoder> encoder, ResourceManager* resourceManager);
        virtual void setPipelineState(PipelineStateId pipelineStateId) override;
        virtual void setShaderBuffer(BufferId bufferId, uint8_t index, ShaderStageFlags stages) override;
        virtual void setShaderTexture(TextureId textureId, uint8_t index, ShaderStageFlags stages) override;
        virtual void drawIndexed(BufferId indexBufferId, uint32_t indexCount, uint32_t indexOffset, uint32_t baseVertexOffset) override;
        virtual void drawPrimitives(uint32_t startOffset, uint32_t vertexCount) override;
        virtual void setVertexBuffer(BufferId vertexBuffer) override;
        
        id<MTLRenderCommandEncoder> getMTLEncoder();
    };
}
