//
//  MetalDevice.h
//  planet
//
//  Created by Eugene Sturm on (8/8/16).
//

#pragma once

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <vector>
#import "DGAssert.h"
#import "MetalResources.h"
#import "MetalShaderLibrary.h"
#import "RenderDevice.h"
#import "ResourceManager.h"
#import "CreateTextureParams.h"

namespace gfx {

    
    class MetalDevice : public RenderDevice {
    private:
        id<MTLDevice>               _device{nil};
        id<MTLCommandQueue>         _queue{nil};
        ResourceManager*            _resourceManager{nullptr};
        MetalShaderLibrary*         _library{nullptr};

        uint64_t _frameDrawCallCount{0};
    public:
        MetalDevice(id<MTLDevice> device, ResourceManager* resourceManager);
        ~MetalDevice();

        // RenderDevice Interface
        virtual RenderDeviceApi GetDeviceApi() override;        
        virtual BufferId AllocateBuffer(const BufferDesc& desc, const void* initialData = nullptr) override;
        virtual VertexLayoutId CreateVertexLayout(const VertexLayoutDesc& desc) override;
        virtual ShaderId GetShader(ShaderType type, const std::string& functionName) override;
        virtual void AddOrUpdateShaders(const std::vector<ShaderData>& shaderData) override;
        virtual PipelineStateId CreatePipelineState(const PipelineStateDesc& desc) override;
        virtual RenderPassId CreateRenderPass(const RenderPassInfo& renderPassInfo) override;
        virtual CommandBuffer* CreateCommandBuffer() override;
        virtual uint8_t* MapMemory(BufferId bufferId, BufferAccess access) override;
        virtual TextureId CreateTexture2D(PixelFormat format, TextureUsageFlags usage, uint32_t width, uint32_t height, void* data, const std::string& debugName = "") override;
        virtual TextureId CreateTextureArray(PixelFormat format, uint32_t levels, uint32_t width, uint32_t height, uint32_t depth, const std::string& debugName = "") override;
        virtual TextureId CreateTextureCube(PixelFormat format, uint32_t width, uint32_t height, void** data, const std::string& debugName = "") override;
        virtual void UpdateTexture(TextureId texture, uint32_t slice, const void* srcData) override;
        virtual void PrintDisplayAdapterInfo() override {};
        virtual void DestroyResource(ResourceId resourceId) override;
        virtual void UnmapMemory(BufferId bufferId) override;
        virtual void Submit(const std::vector<CommandBuffer*>& cmdBuffers) override;
        
        // ----------
        id<MTLDevice> getMTLDevice();
        id<MTLCommandQueue> getMTLCommandQueue();
    private:
        
    private:
        TextureId CreateTexture(const CreateTextureParams& params);
    };
    

    
    
    

}
