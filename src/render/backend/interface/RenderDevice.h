#pragma once

#include <stdint.h>
#include "BufferAccess.h"
#include "BufferDesc.h"
#include "BufferType.h"
#include "BufferUsage.h"
#include "CommandBuffer.h"
#include "Log.h"
#include "PipelineStateDesc.h"
#include "PixelFormat.h"
#include "RenderDeviceApi.h"
#include "ResourceTypes.h"
#include "ShaderDataDesc.h"
#include "ShaderDataType.h"
#include "ShaderFunctionDesc.h"
#include "ShaderLibrary.h"
#include "ShaderType.h"
#include "VertexLayoutDesc.h"
#include "ShaderStageFlags.h"
#include "AttachmentDesc.h"
#include "RenderPassInfo.h"
#include "TextureUsage.h"

namespace gfx {

    struct BackendConfiguration
    {
        std::string DeviceAbbreviation;
        // Shader SubDirectory
        std::string ShaderDir;
        // Extension including dot
        std::string ShaderExtension;
    };
    
    struct DeviceConfiguration {
        std::string DeviceAbbreviation;
        // Shader SubDirectory
        std::string ShaderDir;
        // Extension including dot
        std::string ShaderExtension;
    };

    struct DeviceInitialization {
        void*    windowHandle{0};
        uint32_t windowHeight{0};
        uint32_t windowWidth{0};
        bool     usePrebuiltShaders{false};
    };
    
    class RenderDevice {
    public:
        DeviceConfiguration     DeviceConfig;
        virtual RenderDeviceApi GetDeviceApi()                                   = 0;
        virtual int32_t InitializeDevice(const DeviceInitialization& deviceInit) = 0;
        virtual void ResizeWindow(uint32_t width, uint32_t height) = 0;
        virtual void PrintDisplayAdapterInfo() = 0;

        virtual BufferId AllocateBuffer(const BufferDesc& desc, const void* initialData = nullptr) = 0;
        
        virtual ShaderId GetShader(ShaderType type, const std::string& functionName) = 0;
        virtual void AddOrUpdateShaders(const std::vector<ShaderData>& shaderData) = 0;
                
        virtual PipelineStateId CreatePipelineState(const PipelineStateDesc& desc) = 0;
        virtual RenderPassId CreateRenderPass(const RenderPassInfo& renderPassInfo) = 0;
        
        virtual TextureId CreateTexture2D(PixelFormat format, TextureUsageFlags usage, uint32_t width, uint32_t height, void* data, const std::string& debugName = "") = 0;
        virtual TextureId CreateTextureArray(PixelFormat format, uint32_t levels, uint32_t width, uint32_t height, uint32_t depth, const std::string& debugName = "") = 0;
        virtual TextureId CreateTextureCube(PixelFormat format, uint32_t width, uint32_t height, void** data, const std::string& debugName = "") = 0;
        virtual VertexLayoutId CreateVertexLayout(const VertexLayoutDesc& layoutDesc) = 0;

        virtual void DestroyResource(ResourceId resourceId) = 0;
        
        virtual void Submit(const std::vector<CommandBuffer*>& cmdBuffers) {}

        virtual uint8_t* MapMemory(BufferId buffer, BufferAccess) = 0;
        virtual void UnmapMemory(BufferId buffer) = 0;

        virtual void UpdateTexture(TextureId texture, uint32_t slice, const void* srcData) = 0;
                
        virtual CommandBuffer* CreateCommandBuffer2() { return nullptr; }
        
        virtual uint32_t DrawCallCount() = 0;
    };
}
