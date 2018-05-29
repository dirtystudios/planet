#pragma once

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <vector>
#import "DGAssert.h"
#import "MetalResources.h"
#import "MetalShaderLibrary.h"
#import "MetalView.h"
#import "RenderDelegate.h"
#import "RenderDevice.h"
#import "ResourceManager.h"

namespace gfx {
    struct CreateTextureParams {
        std::string     debugName {""};
        PixelFormat     format{PixelFormat::R8Unorm};
        uint32_t        width{0};
        uint32_t        height{0};
        uint32_t        depth{1};
        uint32_t        mips{1};
        uint32_t        sampleCount{1};
        uint32_t        arrayLength{1};
        MTLCPUCacheMode cpuCacheMode{MTLCPUCacheModeDefaultCache};
        MTLStorageMode  storageMode{MTLStorageModeManaged};
        MTLTextureType  textureType{MTLTextureType2D};
        void* const*    srcData{nullptr};
        uint32_t        srcDataCount{0};
    };
    
    class MetalDevice : public RenderDevice, public RenderDelegate {
    private:
        id<MTLDevice>               _device{nil};
        id<MTLCommandQueue>         _queue{nil};
        ResourceManager*            _resourceManager{nullptr};
        MetalShaderLibrary*         _library{nullptr};

        uint64_t _frameDrawCallCount{0};
    public:
        MetalDevice(id<MTLDevice> device, ResourceManager* resourceManager);
        MetalDevice();
        ~MetalDevice();

        // RenderDevice Interface
        RenderDeviceApi GetDeviceApi();
        int32_t InitializeDevice(const DeviceInitialization& deviceInit);
        BufferId AllocateBuffer(const BufferDesc& desc, const void* initialData = nullptr);
        VertexLayoutId CreateVertexLayout(const VertexLayoutDesc& desc);
        ShaderId GetShader(ShaderType type, const std::string& functionName);
        void AddOrUpdateShaders(const std::vector<ShaderData>& shaderData);
        PipelineStateId CreatePipelineState(const PipelineStateDesc& desc);
        CommandBuffer* CreateCommandBuffer();
        CmdBuffer* CreateCommandBuffer2();
        void Submit(const std::vector<CommandBuffer*>& cmdBuffers);
        uint8_t* MapMemory(BufferId bufferId, BufferAccess access);
        void RenderFrame();
        void ResizeWindow(uint32_t width, uint32_t height);
        TextureId CreateTexture2D(PixelFormat format, uint32_t width, uint32_t height, void* data, const std::string& debugName = "");
        TextureId CreateTextureArray(PixelFormat format, uint32_t levels, uint32_t width, uint32_t height, uint32_t depth, const std::string& debugName = "");
        TextureId CreateTextureCube(PixelFormat format, uint32_t width, uint32_t height, void** data, const std::string& debugName = "");
        void UpdateTexture(TextureId texture, uint32_t slice, const void* srcData);
        void PrintDisplayAdapterInfo() {}
        void DestroyResource(ResourceId resourceId) {}
        void UnmapMemory(BufferId bufferId);

        virtual void Submit(const FrameBuffer& frameBuffer, CommandBuffer** commandBuffers, size_t bufferCount);
        virtual void Submit(const std::vector<CmdBuffer*>& cmdBuffers);
        // RenderDelegate Interface
        void SubmitToGPU();
        uint32_t DrawCallCount();
        
        // ----------
        id<MTLDevice> getMTLDevice();
        id<MTLCommandQueue> getMTLCommandQueue();
    private:
        void submit(id<MTLRenderCommandEncoder> encoder, const DrawItem* drawItem);
        
    private:
        TextureId CreateTexture(const CreateTextureParams& params);
    };
    

    
    
    

}
