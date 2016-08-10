#pragma once

#include "PipelineStateDesc.h"
#include "VertexLayoutDesc.h"
#include <stdint.h>
#include "ResourceTypes.h"
#include "BufferType.h"
#include "BufferUsage.h"
#include "TextureFormat.h"
#include "ShaderType.h"
#include "BufferAccess.h"
#include "Log.h"
#include "CommandBuffer.h"
#include "BufferDesc.h"
#include "Helpers.h"
#include <array>
#include "ShaderDataType.h"
#include "ShaderFunctionDesc.h"
#include "ShaderDataDesc.h"
#include "ShaderLibrary.h"

namespace gfx {

enum class RenderDeviceApi : uint8_t { Unknown = 0, OpenGL, Metal, D3D11 };

static RenderDeviceApi ApiFromString(const std::string& apiString) {
    static constexpr size_t d3d11StringCount                                = 1;
    static constexpr std::array<const char*, d3d11StringCount> d3d11Strings = {{"directx11"}};
    static constexpr size_t glStringCount                                   = 1;
    static constexpr std::array<const char*, glStringCount> glStrings       = {{"opengl"}};
    static constexpr size_t mtlStringCount                                  = 1;
    static constexpr std::array<const char*, mtlStringCount> mtlStrings     = {{"metal"}};

    std::string lowerCase = ToLowercase(apiString);

    for (const char* str : d3d11Strings) {
        if (lowerCase.compare(str) == 0) {
            return RenderDeviceApi::D3D11;
        }
    }

    for (const char* str : glStrings) {
        if (lowerCase.compare(str) == 0) {
            return RenderDeviceApi::OpenGL;
        }
    }

    for (const char* str : mtlStrings) {
        if (lowerCase.compare(str) == 0) {
            return RenderDeviceApi::Metal;
        }
    }

    return RenderDeviceApi::Unknown;
}
struct DeviceConfiguration {
    std::string DeviceAbbreviation;
    // Extension including dot
    std::string ShaderExtension;
};

struct DeviceInitialization {
    void* windowHandle{0};
    uint32_t windowHeight{0};
    uint32_t windowWidth{0};
    bool usePrebuiltShaders{false};
};

class RenderDevice {
public:
    DeviceConfiguration DeviceConfig;

    virtual int32_t InitializeDevice(const DeviceInitialization& deviceInit) = 0;
    virtual void ResizeWindow(uint32_t width, uint32_t height) = 0;
    virtual void PrintDisplayAdapterInfo() = 0;

    virtual BufferId AllocateBuffer(const BufferDesc& desc, const void* initialData = nullptr) = 0;
    
    [[deprecated("Use ShaderLibraries instead")]]
    virtual ShaderId CreateShader(const ShaderFunctionDesc& funcDesc, const ShaderData& data) = 0;
    
    virtual ShaderLibrary* CreateShaderLibrary(const std::vector<ShaderDataDesc>& shaderData) = 0;
    
    [[deprecated("Use Uniform Buffers instead. Keeping around for future push contants")]]
    virtual ShaderParamId CreateShaderParam(ShaderId shader, const char* param, ParamType paramType) = 0;
    
    virtual PipelineStateId CreatePipelineState(const PipelineStateDesc& desc) = 0;
    virtual TextureId CreateTexture2D(TextureFormat format, uint32_t width, uint32_t height, void* data) = 0;
    virtual TextureId CreateTextureArray(TextureFormat format, uint32_t levels, uint32_t width, uint32_t height,
                                         uint32_t depth) = 0;

    virtual TextureId CreateTextureCube(TextureFormat format, uint32_t width, uint32_t height, void** data) = 0;
    virtual VertexLayoutId CreateVertexLayout(const VertexLayoutDesc& layoutDesc) = 0;

    virtual void DestroyResource(ResourceId resourceId) = 0;

    virtual CommandBuffer* CreateCommandBuffer() = 0;
    virtual void Submit(const std::vector<CommandBuffer*>& cmdBuffers) = 0;

    virtual uint8_t* MapMemory(BufferId buffer, BufferAccess) = 0;
    virtual void UnmapMemory(BufferId buffer) = 0;

    virtual void RenderFrame() = 0;
};
}
