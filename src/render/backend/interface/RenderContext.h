//
//  RenderContext.h
//  planet
//
//  Created by Eugene Sturm on 4/19/18.
//

#ifndef RenderContext_h
#define RenderContext_h

#include "ResourceTypes.h"

namespace gfx
{
    struct RenderPassInfo
    {
        std::array<TextureId, 4> colorAttachments { { 0, 0, 0, 0} };
        TextureId depthAttachment { 0 };
        TextureId stencilAttachment { 0 };
        
        std::array<LoadAction, 6> loadActions;
        std::array<StoreAction, 6> storeActions;
    };
    
    class RenderContext
    {
    public:
        virtual void beginRenderPass(const RenderPassInfo& passInfo) = 0;
        virtual void endRenderPass() = 0;
        
        virtual void setVertexStageTexture(TextureId texture, uint32_t index) = 0;
        virtual void setVertexStageBuffer(BufferId buffer, uint32_t index) = 0;
        virtual void setFragmentStageTexture(TextureId texture, uint32_t index) = 0;
        virtual void setFragmentStageBuffer(BufferId buffer, uint32_t index) = 0;
        
        virtual void setInputLayout(VertexLayoutId inputLayer) = 0;
        virtual void setIndexBuffer(BufferId indexBuffer) = 0;
        virtual void setVertexBuffer(BufferId vertexBuffer) = 0;
        
        virtual void setVertexShader(ShaderId vertexShader) = 0;
        virtual void setFragmentShader(ShaderId fragmentShader) = 0;
        
        virtual void drawIndexed(uint32_t primitiveCount, uint32_t startOffset, uint32_t baseVertexOffset) = 0;
    }
}


#endif /* RenderContext_h */
