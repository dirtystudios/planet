#import "MetalDevice.h"
#import <QuartzCore/CVDisplayLink.h>
#include "DGAssert.h"
#include "DrawItemDecoder.h"
#include "Log.h"
#include "MetalEnumAdapter.h"
#include "MetalShaderLibrary.h"
#import "MetalView.h"
#include "TexConvert.h"
#include "MetalCommandBuffer.h"

static const std::string kMetalGfxChannel = "MetalDevice";
#define GFXLog_D(fmt, ...) LOG(Log::Level::Debug, kMetalGfxChannel, fmt, ##__VA_ARGS__)
#define GFXLog_W(fmt, ...) LOG(Log::Level::Warn, kMetalGfxChannel, fmt, ##__VA_ARGS__)
#import "MetalView.h"

using namespace gfx;

id<MTLSamplerState> GetDefaultSampler(id<MTLDevice> device)
{
    static id<MTLSamplerState> sampler = nil;

    if (!sampler) {
        MTLSamplerDescriptor* sd = [[MTLSamplerDescriptor alloc] init];
        sd.sAddressMode          = MTLSamplerAddressModeClampToEdge;
        sd.tAddressMode          = MTLSamplerAddressModeClampToEdge;
        sd.rAddressMode          = MTLSamplerAddressModeClampToEdge;
        sd.minFilter             = MTLSamplerMinMagFilterLinear;
        sd.magFilter             = MTLSamplerMinMagFilterLinear;
        sd.mipFilter             = MTLSamplerMipFilterNotMipmapped;
        sd.maxAnisotropy         = 1;
        sd.normalizedCoordinates = YES;

        sampler = [device newSamplerStateWithDescriptor:sd];
        [sd release];
    }

    return sampler;
}

uint32_t ComputeBytesPerRow(MTLPixelFormat format, uint32_t width)
{
    switch (format) {
        case MTLPixelFormatR8Unorm:
            return width;
        case MTLPixelFormatR32Float:
        case MTLPixelFormatRGBA8Unorm:
            return 4 * width;
        case MTLPixelFormatRGBA32Float:
            return 16 * width;
        case MTLPixelFormatDepth32Float:
            return 16 * width;
        default:
            dg_assert_fail_nm();
    }
    return 0;
}

constexpr bool isDepthFormat(MTLPixelFormat pixelFormat)
{
    return pixelFormat == MTLPixelFormatDepth16Unorm || pixelFormat == MTLPixelFormatDepth32Float || pixelFormat == MTLPixelFormatDepth24Unorm_Stencil8
    || pixelFormat == MTLPixelFormatDepth32Float_Stencil8;
}

constexpr bool isStencilFormat(MTLPixelFormat pixelFormat)
{
    return pixelFormat == MTLPixelFormatStencil8 || pixelFormat == MTLPixelFormatX24_Stencil8 || pixelFormat == MTLPixelFormatX32_Stencil8
    || pixelFormat == MTLPixelFormatDepth24Unorm_Stencil8 || pixelFormat == MTLPixelFormatDepth32Float_Stencil8;
}

MetalDevice::MetalDevice(id<MTLDevice> device, ResourceManager* resourceManager)
{
    _device = device;
    _queue  = [_device newCommandQueue];
    _resourceManager = resourceManager;
    _library = new MetalShaderLibrary(_resourceManager);
}

MetalDevice::MetalDevice() {
    DeviceConfig.ShaderExtension    = ".metal";
    DeviceConfig.DeviceAbbreviation = "metal";
    DeviceConfig.ShaderDir          = "metal";
}

MetalDevice::~MetalDevice() {}

RenderDeviceApi MetalDevice::GetDeviceApi() { return RenderDeviceApi::Metal; }

int32_t MetalDevice::InitializeDevice(const DeviceInitialization& deviceInit) {
//    _window        = reinterpret_cast<NSWindow*>(deviceInit.windowHandle);
//    _view          = [[MetalView alloc] initWithFrame:_window.contentView.frame];
//    _view.delegate = this;
//    [_window.contentView addSubview:_view];
//
//    _device = _view.device;
//    _queue  = [_device newCommandQueue];
//
//    _view.depthPixelFormat = MTLPixelFormatDepth32Float;
//    _inflightSemaphore     = dispatch_semaphore_create(1);
   

    return 0;
}

BufferId MetalDevice::AllocateBuffer(const BufferDesc& desc, const void* initialData) {
    id<MTLBuffer>      mtlBuffer = nullptr;
    MTLResourceOptions options   = 0;

    if(desc.isDynamic) {
        GFXLog_W("Dynamic buffers are unsupported");
    }

    // TODO right now we just treat transient buffers as persistent.
    // just check exact configurations for now
    if (desc.accessFlags == (desc.accessFlags & BufferAccessFlags::GpuReadCpuWriteBits)) {
        options = MTLResourceStorageModeManaged; // currently all buffers are managed for simplicity. not a good idea in the long run
    } else if (desc.accessFlags == (desc.accessFlags & BufferAccessFlags::GpuReadBit)) {
        options = MTLResourceStorageModePrivate;
    } else {
        dg_assert_fail("unsupported access flags");
    }

    dg_assert(!(initialData && options == MTLResourceStorageModePrivate), "not implemented");

    size_t bufferSize = std::max<size_t>(256, desc.size);
    bufferSize += (bufferSize % 16 != 0) ? 16 - (bufferSize % 16) : 0;

    if (initialData) {
        mtlBuffer = [_device newBufferWithBytes:initialData length:bufferSize options:options];
    } else {
        mtlBuffer = [_device newBufferWithLength:bufferSize options:options];
    }

    MetalBuffer* buffer = new MetalBuffer();
    buffer->desc        = desc;
    buffer->desc.size   = bufferSize;
    buffer->mtlBuffer   = mtlBuffer;
    buffer->mtlBuffer.label = [NSString stringWithUTF8String: desc.debugName.c_str()];

    return _resourceManager->AddResource(buffer);
}

VertexLayoutId MetalDevice::CreateVertexLayout(const VertexLayoutDesc& desc) {
    MTLVertexDescriptor* vertexDesc = [MTLVertexDescriptor vertexDescriptor];

    uint32_t idx    = 0;
    size_t   offset = 0;
    for (const VertexLayoutElement& attribute : desc.elements) {
        vertexDesc.attributes[idx].format      = MetalEnumAdapter::toMTL(attribute.type, attribute.storage);
        vertexDesc.attributes[idx].offset      = offset;
        vertexDesc.attributes[idx].bufferIndex = 0;

        idx++;
        offset += GetByteCount(attribute);
        //        dg_assert_equals_nm(alignof(offset), 4);
    }

    vertexDesc.layouts[0].stride       = offset;
    vertexDesc.layouts[0].stepRate     = 1;
    vertexDesc.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;

    MetalVertexLayout* vertLayout = new MetalVertexLayout();
    vertLayout->layoutDesc        = desc;
    vertLayout->mtlVertexDesc     = vertexDesc;

    return _resourceManager->AddResource(vertLayout);
}

PipelineStateId MetalDevice::CreatePipelineState(const PipelineStateDesc& desc) {
    static std::unordered_map<size_t, PipelineStateId> cache; // hobo cache

    size_t h  = std::hash<PipelineStateDesc>()(desc);
    auto   it = cache.find(h);
    if (it != end(cache))
        return it->second;
    
    MetalRenderPass* renderPass = _resourceManager->GetResource<MetalRenderPass>(desc.renderPass);
    dg_assert_nm(renderPass);
    if (renderPass == nullptr) {
        return NULL_ID;
    }
    
    MTLRenderPipelineDescriptor* rpd = [[MTLRenderPipelineDescriptor alloc] init];

    MetalVertexLayout* vertexLayout             = _resourceManager->GetResource<MetalVertexLayout>(desc.vertexLayout);
    rpd.vertexDescriptor                        = vertexLayout->mtlVertexDesc;
    rpd.vertexFunction                          = _resourceManager->GetResource<MetalLibraryFunction>(desc.vertexShader)->mtlFunction;
    rpd.fragmentFunction                        = _resourceManager->GetResource<MetalLibraryFunction>(desc.pixelShader)->mtlFunction;
    
    if (renderPass->info.hasDepth) {
        rpd.depthAttachmentPixelFormat = MetalEnumAdapter::toMTL(renderPass->info.depthAttachment.format);
    }
    
    if (renderPass->info.hasStencil) {
        rpd.stencilAttachmentPixelFormat = MetalEnumAdapter::toMTL(renderPass->info.stencilAttachment.format);
    }
    
    for (int i = 0; i < renderPass->info.attachmentCount; ++i) {
        rpd.colorAttachments[i].pixelFormat         = MetalEnumAdapter::toMTL(renderPass->info.attachments[i].format);
        
        // TODO: blendstate per attachment
        rpd.colorAttachments[i].blendingEnabled     = desc.blendState.enable;
        rpd.colorAttachments[i].alphaBlendOperation = MetalEnumAdapter::toMTL(desc.blendState.alphaMode);
        rpd.colorAttachments[i].rgbBlendOperation   = MetalEnumAdapter::toMTL(desc.blendState.rgbMode);
        //    rpd.colorAttachments[0].writeMask =
        rpd.colorAttachments[i].destinationAlphaBlendFactor = MetalEnumAdapter::toMTL(desc.blendState.dstAlphaFunc);
        rpd.colorAttachments[i].destinationRGBBlendFactor   = MetalEnumAdapter::toMTL(desc.blendState.dstRgbFunc);
        rpd.colorAttachments[i].sourceAlphaBlendFactor      = MetalEnumAdapter::toMTL(desc.blendState.srcAlphaFunc);
        rpd.colorAttachments[i].sourceRGBBlendFactor        = MetalEnumAdapter::toMTL(desc.blendState.srcRgbFunc);
    }
    
    
    NSError*                     error            = nil;
    MTLRenderPipelineReflection* reflection       = nil;
    MTLPipelineOption            options          = MTLPipelineOptionBufferTypeInfo | MTLPipelineOptionArgumentInfo;
    id<MTLRenderPipelineState>   mtlPipelineState = [_device newRenderPipelineStateWithDescriptor:rpd options:options reflection:&reflection error:&error];
    dg_assert(error == nil, "Failed to create pipeline state:%s", [[error localizedDescription] UTF8String]);

    MTLDepthStencilDescriptor* dsd = [[MTLDepthStencilDescriptor alloc] init];
    dsd.depthWriteEnabled          = desc.depthState.enable;
    dsd.depthCompareFunction       = MetalEnumAdapter::toMTL(desc.depthState.depthFunc);

    id<MTLDepthStencilState> mtlDepthStencilState = [_device newDepthStencilStateWithDescriptor:dsd];

    MetalPipelineState* pipelineState   = new MetalPipelineState();
    pipelineState->mtlPipelineState     = mtlPipelineState;
    pipelineState->mtlDepthStencilState = mtlDepthStencilState;
    pipelineState->pipelineStateDesc    = desc;
    pipelineState->reflection           = reflection;

    [rpd release];
    [dsd release];

    PipelineStateId pipelineStateId = _resourceManager->AddResource(pipelineState);
    cache.insert({h, pipelineStateId});

    return pipelineStateId;
}
ShaderId MetalDevice::GetShader(ShaderType type, const std::string& functionName) { return _library->GetShader(type, functionName); }

void MetalDevice::AddOrUpdateShaders(const std::vector<ShaderData>& datas) {
    dg_assert_nm(datas.size() > 0);

    ShaderDataType expectedType = datas[0].type;
    auto resultIt               = std::find_if_not(begin(datas), end(datas), [&](const ShaderData& data) { return data.type == expectedType; });
    dg_assert(resultIt == end(datas), "dont support differet data types in same shader library");

    MTLCompileOptions* options = [[MTLCompileOptions alloc] init];
    options.languageVersion    = MTLLanguageVersion1_1;

    for (const ShaderData& data : datas) {
        NSError*       error      = nil;
        id<MTLLibrary> mtlLibrary = nil;

        if (data.type == ShaderDataType::Source) {
            const char* str  = reinterpret_cast<const char*>(data.data);
            NSString* source = [NSString stringWithCString:str encoding:[NSString defaultCStringEncoding]];
            mtlLibrary       = [_device newLibraryWithSource:source options:options error:&error];
            dg_assert(error == nil, "%s", [[error localizedDescription] UTF8String]);
        } else {
            dispatch_data_t binary = dispatch_data_create(data.data, data.len, nullptr, nullptr);
            mtlLibrary             = [_device newLibraryWithData:binary error:&error];
            dg_assert(error == nil, "%s", [[error localizedDescription] UTF8String]);
        }
        _library->AddLibrary(mtlLibrary);
    }
    [options release];
}

TextureId MetalDevice::CreateTexture(const CreateTextureParams& params) {
    MTLPixelFormat mtlPixelFormat = MetalEnumAdapter::toMTL(params.format);

    bool convert = false;
    if (params.format == PixelFormat::RGB8Unorm) {
        if (params.srcData) {
            mtlPixelFormat = MTLPixelFormatRGBA8Unorm;
            convert        = true;
        } else {
            dg_assert_fail("24 bit texture unsupported");
        }
    }

    const bool requireStorageModePrivate = isDepthFormat(mtlPixelFormat) || isStencilFormat(mtlPixelFormat);
    
    MTLTextureDescriptor* desc = [[MTLTextureDescriptor alloc] init];
    desc.textureType           = params.textureType;
    desc.width                 = params.width;
    desc.height                = params.height;
    desc.pixelFormat           = mtlPixelFormat;
    desc.depth                 = params.depth;
    desc.mipmapLevelCount      = params.mips;
    desc.sampleCount           = params.sampleCount;
    desc.arrayLength           = params.arrayLength;
    desc.cpuCacheMode          = params.cpuCacheMode;
    desc.storageMode           = requireStorageModePrivate ? MTLStorageModePrivate : params.storageMode;
    desc.usage                 = params.usage;

    MetalTexture* texture    = new MetalTexture();
    texture->mtlTexture      = [_device newTextureWithDescriptor:desc];
    texture->mtlSamplerState = GetDefaultSampler(_device);
    texture->externalFormat  = params.format;

    void* const* srcDatas      = params.srcData;
    uint32_t     width         = params.width;
    uint32_t     height        = params.height;
    uint32_t     bytesPerRow   = ComputeBytesPerRow(mtlPixelFormat, width);
    uint32_t     bytesPerImage = bytesPerRow * height;
    if (srcDatas) {
        dg_assert_nm(bytesPerRow != 0);

        const uint8_t* srcData = nullptr;
        if (convert) {
            srcData = new uint8_t[bytesPerImage];
        }
        for (uint32_t idx = 0; idx < params.srcDataCount; ++idx) {
            if (convert) {
                Convert24BitTo32Bit(reinterpret_cast<uintptr_t>(srcDatas[idx]), reinterpret_cast<uintptr_t>(srcData), width * height);
            } else {
                srcData = reinterpret_cast<uint8_t*>(srcDatas[idx]);
            }

            MTLRegion region = MTLRegionMake2D(0, 0, width, height);
            [texture->mtlTexture replaceRegion:region mipmapLevel:0 slice:idx withBytes:srcData bytesPerRow:bytesPerRow bytesPerImage:bytesPerImage];
        }

        if (convert) {
            delete[] srcData;
        }
    }
    texture->bytesPerRow   = bytesPerRow;
    texture->bytesPerImage = bytesPerImage;
    texture->mtlTexture.label = [NSString stringWithUTF8String:params.debugName.c_str()];

    [desc release];
    return _resourceManager->AddResource(texture);
}

TextureId MetalDevice::CreateTexture2D(PixelFormat format, TextureUsageFlags usage, uint32_t width, uint32_t height, void* srcData, const std::string& debugName) {
    CreateTextureParams params;
    params.debugName   = debugName;
    params.format      = format;
    params.width       = width;
    params.height      = height;
    params.textureType = MTLTextureType2D;
    params.usage       = MetalEnumAdapter::toMTL(usage);
    if (srcData) {
        params.srcData      = &srcData;
        params.srcDataCount = 1;
    }

    return CreateTexture(params);
}
TextureId MetalDevice::CreateTextureArray(PixelFormat format, uint32_t levels, uint32_t width, uint32_t height, uint32_t depth, const std::string& debugName) {
    CreateTextureParams params;
    params.debugName   = debugName;
    params.format      = format;
    params.width       = width;
    params.height      = height;
    params.arrayLength = depth;
    params.mips        = levels;
    params.textureType = MTLTextureType2DArray;

    return CreateTexture(params);
}
TextureId MetalDevice::CreateTextureCube(PixelFormat format, uint32_t width, uint32_t height, void** data, const std::string& debugName) {
    CreateTextureParams params;
    params.debugName   = debugName;
    params.format      = format;
    params.width       = width;
    params.height      = height;
    params.textureType = MTLTextureTypeCube;
    if (data) {
        params.srcData      = data;
        params.srcDataCount = 6;
    }

    return CreateTexture(params);
}

void MetalDevice::UpdateTexture(TextureId textureId, uint32_t slice, const void* srcData) {
    dg_assert_nm(slice >= 0 && srcData != nullptr);
    // TOOD: doesnt properly support 24 bit textures
    MetalTexture* texture = _resourceManager->GetResource<MetalTexture>(textureId);
    dg_assert_nm(texture);

    MTLRegion region = MTLRegionMake2D(0, 0, texture->mtlTexture.width, texture->mtlTexture.height);
    [texture->mtlTexture replaceRegion:region mipmapLevel:0 slice:slice withBytes:srcData bytesPerRow:texture->bytesPerRow bytesPerImage:texture->bytesPerImage];
}

uint8_t* MetalDevice::MapMemory(BufferId bufferId, BufferAccess access) {
    MetalBuffer* buffer = _resourceManager->GetResource<MetalBuffer>(bufferId);
    if (!(buffer->desc.accessFlags & BufferAccessFlags::CpuWriteBit)) {
        return nullptr;
    }

    void* ptr = [buffer->mtlBuffer contents];
    dg_assert_nm(ptr);
    return reinterpret_cast<uint8_t*>(ptr);
}

void MetalDevice::UnmapMemory(BufferId bufferId) {
    MetalBuffer* buffer = _resourceManager->GetResource<MetalBuffer>(bufferId);
    if (!buffer) {
        return;
    }
    // currently all buffers are managed so we need to invalidate on update
    NSRange range = NSMakeRange(0, buffer->desc.size);
    [buffer->mtlBuffer didModifyRange:range];
}

CommandBuffer* MetalDevice::CreateCommandBuffer2()
{
    // TODO: manage this
    return new MetalCommandBuffer([_queue commandBuffer], _resourceManager);
}

void MetalDevice::Submit(const std::vector<CommandBuffer*>& cmdBuffers)
{
    for (CommandBuffer* cmdBuffer : cmdBuffers) {
        MetalCommandBuffer* metalCommandBuffer = reinterpret_cast<MetalCommandBuffer*>(cmdBuffer);
        metalCommandBuffer->commit();
    }
}

RenderPassId MetalDevice::CreateRenderPass(const RenderPassInfo& renderPassInfo)
{
    MetalRenderPass* renderPass = new MetalRenderPass();
    renderPass->info = renderPassInfo;
    return _resourceManager->AddResource(renderPass);
}

uint32_t MetalDevice::DrawCallCount() { return _frameDrawCallCount; }

void MetalDevice::ResizeWindow(uint32_t width, uint32_t height) { }

id<MTLDevice> MetalDevice::getMTLDevice()
{
    return _device;
}

id<MTLCommandQueue> MetalDevice::getMTLCommandQueue()
{
    return _queue;
}

