#import "MetalDevice.h"
#import <QuartzCore/CVDisplayLink.h>
#include "DGAssert.h"
#include "DrawItemDecoder.h"
#include "Log.h"
#include "MetalEnumAdapter.h"
#include "MetalShaderLibrary.h"
#import "MetalView.h"
#include "TexConvert.h"

static const std::string kMetalGfxChannel = "metal";
#define GFXLog(fmt, ...) LOG(Log::Debug, kMetalGfxChannel, category, fmt, ...)
#import "MetalView.h"

namespace gfx {

id<MTLSamplerState> GetDefaultSampler(id<MTLDevice> device) {
    static id<MTLSamplerState> sampler = nil;

    if (!sampler) {
        MTLSamplerDescriptor* sd = [[MTLSamplerDescriptor alloc] init];
        sd.sAddressMode          = MTLSamplerAddressModeClampToEdge;
        sd.tAddressMode          = MTLSamplerAddressModeClampToEdge;
        sd.rAddressMode          = MTLSamplerAddressModeClampToEdge;
        sd.minFilter             = MTLSamplerMinMagFilterNearest;
        sd.magFilter             = MTLSamplerMinMagFilterNearest;
        sd.mipFilter             = MTLSamplerMipFilterNotMipmapped;
        sd.maxAnisotropy         = 1;
        sd.normalizedCoordinates = YES;

        sampler = [device newSamplerStateWithDescriptor:sd];
        [sd release];
    }

    return sampler;
}

uint32_t ComputeBytesPerRow(MTLPixelFormat format, uint32_t width) {
    switch (format) {
        case MTLPixelFormatR8Unorm:
            return width;
        case MTLPixelFormatR32Float:
        case MTLPixelFormatRGBA8Unorm:
            return 4 * width;
        case MTLPixelFormatRGBA32Float:
            return 16 * width;
        default:
            dg_assert_fail_nm();
    }
    return 0;
}

MetalDevice::MetalDevice() {
    DeviceConfig.ShaderExtension    = ".metal";
    DeviceConfig.DeviceAbbreviation = "metal";
    DeviceConfig.ShaderDir          = "metal";
}

MetalDevice::~MetalDevice() {}

RenderDeviceApi MetalDevice::GetDeviceApi() { return RenderDeviceApi::Metal; }

int32_t MetalDevice::InitializeDevice(const DeviceInitialization& deviceInit) {
    _window        = reinterpret_cast<NSWindow*>(deviceInit.windowHandle);
    _view          = [[MetalView alloc] initWithFrame:_window.contentView.frame];
    _view.delegate = this;
    [_window.contentView addSubview:_view];

    _device = _view.device;
    _queue  = [_device newCommandQueue];

    _view.depthPixelFormat = MTLPixelFormatDepth32Float;
    _inflightSemaphore     = dispatch_semaphore_create(1);
    _library               = new MetalShaderLibrary(&_resourceManager);

    return 0;
}

BufferId MetalDevice::AllocateBuffer(const BufferDesc& desc, const void* initialData) {
    id<MTLBuffer>      mtlBuffer = nullptr;
    MTLResourceOptions options   = 0;

    dg_assert(!desc.isDynamic, "not implemented");

    // TODO right now we just treat transient buffers as persistent.
    // just check exact configurations for now
    if (desc.accessFlags == (desc.accessFlags & BufferAccessFlags::GpuReadCpuWriteBits)) {
        options = MTLResourceOptionCPUCacheModeWriteCombined;
    } else if (desc.accessFlags == (desc.accessFlags & BufferAccessFlags::GpuReadBit)) {
        options = MTLResourceStorageModePrivate;
    } else {
        dg_assert_fail("unsupported access flags");
    }

    dg_assert(!(initialData && options == MTLResourceStorageModePrivate), "not implemented");

    size_t bufferSize = std::max<size_t>(256, desc.size);

    if (initialData) {
        mtlBuffer = [_device newBufferWithBytes:initialData length:bufferSize options:options];
    } else {
        mtlBuffer = [_device newBufferWithLength:bufferSize options:options];
    }

    MetalBuffer* buffer = new MetalBuffer();
    buffer->desc        = desc;
    buffer->mtlBuffer   = mtlBuffer;

    return _resourceManager.AddResource(buffer);
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

    return _resourceManager.AddResource(vertLayout);
}

PipelineStateId MetalDevice::CreatePipelineState(const PipelineStateDesc& desc) {        
    MTLRenderPipelineDescriptor* rpd = [[MTLRenderPipelineDescriptor alloc] init];

    MetalVertexLayout* vertexLayout             = _resourceManager.GetResource<MetalVertexLayout>(desc.vertexLayout);
    rpd.vertexDescriptor                        = vertexLayout->mtlVertexDesc;
    rpd.vertexFunction                          = _resourceManager.GetResource<MetalLibraryFunction>(desc.vertexShader)->mtlFunction;
    rpd.fragmentFunction                        = _resourceManager.GetResource<MetalLibraryFunction>(desc.pixelShader)->mtlFunction;
    rpd.depthAttachmentPixelFormat              = _view.depthPixelFormat;
    rpd.colorAttachments[0].pixelFormat         = _view.colorPixelFormat;
    rpd.colorAttachments[0].blendingEnabled     = desc.blendState.enable;
    rpd.colorAttachments[0].alphaBlendOperation = MetalEnumAdapter::toMTL(desc.blendState.alphaMode);
    rpd.colorAttachments[0].rgbBlendOperation   = MetalEnumAdapter::toMTL(desc.blendState.rgbMode);
    //    rpd.colorAttachments[0].writeMask =
    rpd.colorAttachments[0].destinationAlphaBlendFactor = MetalEnumAdapter::toMTL(desc.blendState.dstAlphaFunc);
    rpd.colorAttachments[0].destinationRGBBlendFactor   = MetalEnumAdapter::toMTL(desc.blendState.dstRgbFunc);
    rpd.colorAttachments[0].sourceAlphaBlendFactor      = MetalEnumAdapter::toMTL(desc.blendState.srcAlphaFunc);
    rpd.colorAttachments[0].sourceRGBBlendFactor        = MetalEnumAdapter::toMTL(desc.blendState.srcRgbFunc);

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
    
    return _resourceManager.AddResource(pipelineState);
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
    desc.storageMode           = params.storageMode;

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

    [desc release];
    return _resourceManager.AddResource(texture);
}

TextureId MetalDevice::CreateTexture2D(PixelFormat format, uint32_t width, uint32_t height, void* srcData) {
    CreateTextureParams params;
    params.format      = format;
    params.width       = width;
    params.height      = height;
    params.textureType = MTLTextureType2D;
    if (srcData) {
        params.srcData      = &srcData;
        params.srcDataCount = 1;
    }

    return CreateTexture(params);
}
TextureId MetalDevice::CreateTextureArray(PixelFormat format, uint32_t levels, uint32_t width, uint32_t height, uint32_t depth) {
    CreateTextureParams params;
    params.format      = format;
    params.width       = width;
    params.height      = height;
    params.arrayLength = depth;
    params.mips        = levels;
    params.textureType = MTLTextureType2DArray;

    return CreateTexture(params);
}
TextureId MetalDevice::CreateTextureCube(PixelFormat format, uint32_t width, uint32_t height, void** data) {
    CreateTextureParams params;
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
    MetalTexture* texture = _resourceManager.GetResource<MetalTexture>(textureId);
    dg_assert_nm(texture);

    MTLRegion region = MTLRegionMake2D(0, 0, texture->mtlTexture.width, texture->mtlTexture.height);
    [texture->mtlTexture replaceRegion:region mipmapLevel:0 slice:slice withBytes:srcData bytesPerRow:texture->bytesPerRow bytesPerImage:texture->bytesPerImage];
}

uint8_t* MetalDevice::MapMemory(BufferId bufferId, BufferAccess access) {
    MetalBuffer* buffer = _resourceManager.GetResource<MetalBuffer>(bufferId);
    if (!(buffer->desc.accessFlags & BufferAccessFlags::CpuWriteBit)) {
        return nullptr;
    }

    void* ptr = [buffer->mtlBuffer contents];
    dg_assert_nm(ptr);
    return reinterpret_cast<uint8_t*>(ptr);
}

CommandBuffer* MetalDevice::CreateCommandBuffer() { return new CommandBuffer(); }
void MetalDevice::Submit(const std::vector<CommandBuffer*>& cmdBuffers) { _commandBuffers.insert(end(_commandBuffers), begin(cmdBuffers), end(cmdBuffers)); }
void                                                        MetalDevice::RenderFrame() {
    [_view display];
    _commandBuffers.clear();
}

#pragma mark - Rawr

void MetalDevice::SubmitToGPU() {
    dispatch_semaphore_wait(_inflightSemaphore, DISPATCH_TIME_FOREVER);
    id<MTLCommandBuffer> mtlCmdBuff     = [_queue commandBuffer];
    id<MTLRenderCommandEncoder> encoder = [mtlCmdBuff renderCommandEncoderWithDescriptor:_view.renderPassDescriptor];
    [encoder pushDebugGroup:@"BeginFrame"];

    std::vector<VertexStream> streams;
    std::vector<Binding>      bindings;

    for (const CommandBuffer* cmdBuffer : _commandBuffers) {
        const std::vector<const DrawItem*>* items = cmdBuffer->GetDrawItems();

        PipelineStateId pipelineStateId;
        DrawCall        drawCall;
        BufferId        indexBufferId;

        for (const DrawItem* item : *items) {
            DrawItemDecoder decoder(item);

            size_t streamCount = decoder.GetStreamCount();
            dg_assert(streamCount == 1, "> 1 stream count not supported");
            size_t bindingCount = decoder.GetBindingCount();

            streams.clear();
            bindings.clear();
            streams.resize(streamCount);
            bindings.resize(bindingCount);

            VertexStream* streamPtr  = streams.data();
            Binding*      bindingPtr = bindings.data();

            dg_assert_nm(decoder.ReadDrawCall(&drawCall));
            dg_assert_nm(decoder.ReadPipelineState(&pipelineStateId));
            dg_assert_nm(decoder.ReadIndexBuffer(&indexBufferId));
            dg_assert_nm(decoder.ReadVertexStreams(&streamPtr));
            if (bindingCount > 0) {
                dg_assert_nm(decoder.ReadBindings(&bindingPtr));
            }

            MetalPipelineState* pipelineState = _resourceManager.GetResource<MetalPipelineState>(pipelineStateId);
            MetalBuffer*        indexBuffer   = nullptr;
            if (indexBufferId != 0) {
                indexBuffer = _resourceManager.GetResource<MetalBuffer>(indexBufferId);
            }

            [encoder setRenderPipelineState:pipelineState->mtlPipelineState];
            [encoder setDepthStencilState:pipelineState->mtlDepthStencilState];
            [encoder setFrontFacingWinding:MetalEnumAdapter::toMTL(pipelineState->pipelineStateDesc.rasterState.windingOrder)];
            [encoder setCullMode:MetalEnumAdapter::toMTL(pipelineState->pipelineStateDesc.rasterState.cullMode)];
            [encoder setTriangleFillMode:MetalEnumAdapter::toMTL(pipelineState->pipelineStateDesc.rasterState.fillMode)];

            // TODO actually use vertex streams
            MetalBuffer* vertexBuffer = _resourceManager.GetResource<MetalBuffer>(streamPtr[0].vertexBuffer);
            [encoder setVertexBuffer:vertexBuffer->mtlBuffer offset:0 atIndex:0];

            for (const Binding& binding : bindings) {
                if (binding.stageFlags & ShaderStageFlags::VertexBit) {
                    switch (binding.type) {
                        case Binding::Type::ConstantBuffer: {
                            MetalBuffer* constantBuffer = _resourceManager.GetResource<MetalBuffer>(binding.resource);
                            [encoder setVertexBuffer:constantBuffer->mtlBuffer offset:0 atIndex:binding.slot + 1];
                            break;
                        }
                        case Binding::Type::Texture: {
                            MetalTexture* texture = _resourceManager.GetResource<MetalTexture>(binding.resource);
                            [encoder setVertexTexture:texture->mtlTexture atIndex:binding.slot];
                            [encoder setVertexSamplerState:texture->mtlSamplerState atIndex:binding.slot];
                            break;
                        }
                        default:
                            dg_assert_fail_nm();
                    }
                }

                if (binding.stageFlags & ShaderStageFlags::PixelBit) {
                    switch (binding.type) {
                        case Binding::Type::ConstantBuffer: {
                            MetalBuffer* constantBuffer = _resourceManager.GetResource<MetalBuffer>(binding.resource);
                            [encoder setFragmentBuffer:constantBuffer->mtlBuffer offset:0 atIndex:binding.slot + 1];
                            break;
                        }
                        case Binding::Type::Texture: {
                            MetalTexture* texture = _resourceManager.GetResource<MetalTexture>(binding.resource);
                            [encoder setFragmentTexture:texture->mtlTexture atIndex:binding.slot];
                            [encoder setFragmentSamplerState:texture->mtlSamplerState atIndex:binding.slot];
                            break;
                        }
                        default:
                            dg_assert_fail_nm();
                    }
                }
            }

            MTLPrimitiveType primitiveType = MetalEnumAdapter::toMTL(pipelineState->pipelineStateDesc.topology);
            switch (drawCall.type) {
                case DrawCall::Type::Arrays: {
                    [encoder drawPrimitives:primitiveType vertexStart:drawCall.offset vertexCount:drawCall.primitiveCount];
                    break;
                }
                case DrawCall::Type::Indexed: {
                    [encoder drawIndexedPrimitives:primitiveType
                                        indexCount:drawCall.primitiveCount
                                         indexType:MTLIndexTypeUInt32
                                       indexBuffer:indexBuffer->mtlBuffer
                                 indexBufferOffset:drawCall.offset];
                    break;
                }
                default:
                    dg_assert_fail_nm();
            }
        }
    }
    [encoder popDebugGroup];
    [encoder endEncoding];

    __block dispatch_semaphore_t blockSemaphore = _inflightSemaphore;
    [mtlCmdBuff addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
      dispatch_semaphore_signal(blockSemaphore);
    }];

    [mtlCmdBuff presentDrawable:_view.currentDrawable];
    [mtlCmdBuff commit];
}

void MetalDevice::ResizeWindow(uint32_t width, uint32_t height) { [_view shouldResize]; }
}
