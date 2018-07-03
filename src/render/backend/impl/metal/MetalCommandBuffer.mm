//
//  MetalCommandBuffer.mm
//  planet
//
//  Created by Eugene Sturm on 5/25/18.
//

#include "MetalCommandBuffer.h"
#include "ResourceManager.h"
#include "MetalResources.h"
#include "MetalRenderPassCommandBuffer.h"
#include "MetalEnumAdapter.h"

using namespace gfx;

MetalCommandBuffer::MetalCommandBuffer(id<MTLCommandBuffer> commandBuffer, ResourceManager* resourceManager)
: _commandBuffer(commandBuffer)
, _resourceManager(resourceManager)
{}

RenderPassCommandBuffer* MetalCommandBuffer::beginRenderPass(RenderPassId passId, const FrameBuffer& frameBuffer, const std::string& name)
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

void MetalCommandBuffer::endRenderPass(RenderPassCommandBuffer* commandBuffer)
{
    if (commandBuffer != _currentPass.get()) {
        return;
    }
    
    id<MTLRenderCommandEncoder> encoder = _currentPass->getMTLEncoder();
    [encoder endEncoding];
    _currentPass.reset();
}

id<MTLCommandBuffer> MetalCommandBuffer::getMTLCommandBuffer()
{
    return _commandBuffer;
}

void MetalCommandBuffer::commit()
{
    dg_assert_nm(_currentPass == nullptr);
    [_commandBuffer commit];
}

MTLRenderPassDescriptor* MetalCommandBuffer::getMTLRenderPassDescriptor(MetalRenderPass* renderPass, const FrameBuffer& frameBuffer)
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
            colorAttachment.clearColor = MTLClearColorMake(.25, .25, .25, 1);
            
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
