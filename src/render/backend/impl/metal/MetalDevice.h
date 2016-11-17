#pragma once

#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#include <vector>
#include "DGAssert.h"
#include "MetalResources.h"
#include "MetalShaderLibrary.h"
#import "MetalView.h"
#include "RenderDelegate.h"
#include "RenderDevice.h"
#include "ResourceManager.h"

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
    ResourceManager             _resourceManager;
    MetalShaderLibrary*         _library{nullptr};
    std::vector<CommandBuffer*> _commandBuffers;
    dispatch_semaphore_t        _inflightSemaphore;
    NSWindow*                   _window{nil};
    MetalView*                  _view{nil};

    uint64_t _frameDrawCallCount{0};

public:
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

    // RenderDelegate Interface
    void SubmitToGPU();
    uint32_t DrawCallCount();

private:
    TextureId CreateTexture(const CreateTextureParams& params);
};
}
