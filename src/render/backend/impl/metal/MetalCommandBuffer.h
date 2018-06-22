//
//  MetalCommandBuffer.h
//  planet
//
//  Created by Eugene Sturm on 5/25/18.
//

#pragma once

#include <Metal/Metal.h>
#include "RenderDevice.h"
#include "ResourceManager.h"
#import "MetalResources.h"
#import "ShaderStageFlags.h"
#include "MetalEnumAdapter.h"
#include "DGAssert.h"

namespace gfx
{
    
    class MetalRenderPassCommandBuffer : public RenderPassCommandBuffer
    {
    private:
        id<MTLRenderCommandEncoder> _encoder;
        ResourceManager* _resourceManager;
        MetalPipelineState* _currentPipelineState { nullptr };
    public:
        MetalRenderPassCommandBuffer(id<MTLRenderCommandEncoder> encoder, ResourceManager* resourceManager)
        : _encoder(encoder)
        , _resourceManager(resourceManager)
        {}
                        
        void setPipelineState(PipelineStateId pipelineStateId) override
        {
            MetalPipelineState* pipelineState = _resourceManager->GetResource<MetalPipelineState>(pipelineStateId);
            [_encoder setRenderPipelineState:pipelineState->mtlPipelineState];
            
            _currentPipelineState = pipelineState;
            
            [_encoder setDepthStencilState:pipelineState->mtlDepthStencilState];
            [_encoder setFrontFacingWinding:MetalEnumAdapter::toMTL(pipelineState->pipelineStateDesc.rasterState.windingOrder)];
            [_encoder setCullMode:MetalEnumAdapter::toMTL(pipelineState->pipelineStateDesc.rasterState.cullMode)];
            [_encoder setTriangleFillMode:MetalEnumAdapter::toMTL(pipelineState->pipelineStateDesc.rasterState.fillMode)];            
        }
        void setShaderBuffer(BufferId bufferId, uint8_t index, ShaderStageFlags stages) override
        {
            index += 1;
            MetalBuffer* buffer = _resourceManager->GetResource<MetalBuffer>(bufferId);
            if (stages & ShaderStageFlags::VertexBit) {
                [_encoder setVertexBuffer:buffer->mtlBuffer offset:0 atIndex:index];
            }
            if (stages & ShaderStageFlags::PixelBit) {
                [_encoder setFragmentBuffer:buffer->mtlBuffer offset:0 atIndex:index];
            }
            if (stages & ShaderStageFlags::TessEvalBit) {
                // todo
            }
            if (stages & ShaderStageFlags::TessControlBit) {
                // todo
            }
        }
        void setShaderTexture(TextureId textureId, uint8_t index, ShaderStageFlags stages) override
        {
            MetalTexture* texture = _resourceManager->GetResource<MetalTexture>(textureId);
            if (stages & ShaderStageFlags::VertexBit) {
                [_encoder setVertexTexture:texture->mtlTexture atIndex:index];
                [_encoder setVertexSamplerState:texture->mtlSamplerState atIndex:index];
                
            }
            if (stages & ShaderStageFlags::PixelBit) {
                [_encoder setFragmentTexture:texture->mtlTexture atIndex:index];
                [_encoder setFragmentSamplerState:texture->mtlSamplerState atIndex:index];
            }
            if (stages & ShaderStageFlags::TessEvalBit) {
                // todo
            }
            if (stages & ShaderStageFlags::TessControlBit) {
                // todo
            }
        }
        void drawIndexed(BufferId indexBufferId, uint32_t indexCount, uint32_t indexOffset) override
        {
            const MTLPrimitiveType primitiveType = MetalEnumAdapter::toMTL(_currentPipelineState->pipelineStateDesc.topology);
            MetalBuffer* indexBuffer = _resourceManager->GetResource<MetalBuffer>(indexBufferId);
            [_encoder drawIndexedPrimitives:primitiveType
                                indexCount:indexCount
                                 indexType:MTLIndexTypeUInt32
                               indexBuffer:indexBuffer->mtlBuffer
                         indexBufferOffset:indexOffset * sizeof(uint32_t) // has to be in bytes when using this draw call i guess
                             instanceCount:1
                                baseVertex:0
                              baseInstance:1];
        }
        void drawPrimitives(uint32_t startOffset, uint32_t vertexCount)
        {
            const MTLPrimitiveType primitiveType = MetalEnumAdapter::toMTL(_currentPipelineState->pipelineStateDesc.topology);
            [_encoder drawPrimitives:primitiveType
                         vertexStart:startOffset
                         vertexCount:vertexCount];
            
        }
        
        void setVertexBuffer(BufferId vertexBuffer)
        {
            MetalBuffer* buffer = _resourceManager->GetResource<MetalBuffer>(vertexBuffer);
            [_encoder setVertexBuffer:buffer->mtlBuffer offset:0 atIndex:0];
        }                
        
        id<MTLRenderCommandEncoder> getMTLEncoder()
        {
            return _encoder;
        }
    };
    
    class MetalCommandBuffer : public CmdBuffer
    {
    private:
        id<MTLCommandBuffer> _commandBuffer;
        ResourceManager* _resourceManager { nullptr };
        
        std::unique_ptr<MetalRenderPassCommandBuffer> _currentPass { nullptr };
    public:
        MetalCommandBuffer(id<MTLCommandBuffer> commandBuffer, ResourceManager* resourceManager)
        : _commandBuffer(commandBuffer)
        , _resourceManager(resourceManager)
        {}
        
        RenderPassCommandBuffer* beginRenderPass(RenderPassId passId, const FrameBuffer& frameBuffer) override
        {
            if (_currentPass) {
                return nullptr;
            }
            
            
            // TODO: Validate RenderPassId is valid with FrameBuffer
            MetalRenderPass* renderPass = _resourceManager->GetResource<MetalRenderPass>(passId);
            dg_assert_nm(renderPass);
            if (renderPass == nullptr) {
                return nullptr;
            }
            
            MTLRenderPassDescriptor* renderPassDesc = getMTLRenderPassDescriptor(renderPass, frameBuffer);
            
            id<MTLRenderCommandEncoder> encoder = [_commandBuffer renderCommandEncoderWithDescriptor:renderPassDesc];
            _currentPass.reset(new MetalRenderPassCommandBuffer(encoder, _resourceManager));
            
            return _currentPass.get();
        }
        
        void endRenderPass(RenderPassCommandBuffer* commandBuffer) override
        {
            if (commandBuffer != _currentPass.get()) {
                return;
            }
            
            id<MTLRenderCommandEncoder> encoder = _currentPass->getMTLEncoder();
            [encoder endEncoding];
            _currentPass.reset();
        }
        
        id<MTLCommandBuffer> getMTLCommandBuffer()
        {
            return _commandBuffer;
        }
        
        void commit()
        {
            dg_assert_nm(_currentPass == nullptr);
            [_commandBuffer commit];
        }
        
        MTLRenderPassDescriptor* getMTLRenderPassDescriptor(MetalRenderPass* renderPass, const FrameBuffer& frameBuffer)
        {
            MTLRenderPassDescriptor* renderPassDesc = [MTLRenderPassDescriptor new];
            
            const RenderPassInfo& info = renderPass->info;
            dg_assert_nm(info.attachmentCount == frameBuffer.colorCount);
            for (int i = 0; i < info.attachmentCount; ++i) {
                const AttachmentDesc& attachment = info.attachments[i];
                
                MetalTexture* texture = _resourceManager->GetResource<MetalTexture>(frameBuffer.color[i]);
                dg_assert_nm(texture);
                if (texture) {
                    dg_assert_nm(attachment.format == texture->externalFormat);
                    MTLRenderPassColorAttachmentDescriptor* colorAttachment = [MTLRenderPassColorAttachmentDescriptor new];
                    colorAttachment.texture = texture->mtlTexture;;
                    colorAttachment.loadAction = MetalEnumAdapter::toMTL(attachment.loadAction);
                    colorAttachment.storeAction = MetalEnumAdapter::toMTL(attachment.storeAction);
                    //colorAttachment.clearColor = attachmentDesc.clearColor;
                    
                    [renderPassDesc.colorAttachments setObject:colorAttachment atIndexedSubscript:i];
                }                
            }
            
            if (info.hasDepth) {
                MetalTexture* texture = _resourceManager->GetResource<MetalTexture>(frameBuffer.depth);
                const AttachmentDesc& attachment = info.depthAttachment;
                if (texture) {
                    MTLRenderPassDepthAttachmentDescriptor* depthAttachment = [MTLRenderPassDepthAttachmentDescriptor new];
                    depthAttachment.texture = texture->mtlTexture;
                    depthAttachment.loadAction = MetalEnumAdapter::toMTL(attachment.loadAction);
                    depthAttachment.storeAction = MetalEnumAdapter::toMTL(attachment.storeAction);
                    //depthAttachment.clearDepth = attachmentDesc.clearDepth;
                    
                    renderPassDesc.depthAttachment = depthAttachment;
                }
            }
            
            dg_assert(info.hasStencil == false, "TODO");
            
            return renderPassDesc;
        }
    };
    
    
}
