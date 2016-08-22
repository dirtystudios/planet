#pragma once

#include <array>
#include <list>
#include <unordered_map>
#include "Binding.h"
#include "CommandBuffer.h"
#include "DMath.h"
#include "SimpleShaderLibrary.h"
#include "ResourceManager.h"
#include "GLContext.h"
#include "GLStructs.h"
#include "Pool.h"
#include "RenderDevice.h"
#include "ResourceId.h"
#include "VertexStream.h"

namespace gfx {
class GLDevice : public RenderDevice {
private:
    using GLVaoCacheKey                            = size_t;
    using GLVaoCache                               = std::unordered_map<GLVaoCacheKey, GLVertexArrayObject*>;
    constexpr static size_t kCommandBufferPoolSize = 32;

private:
    GLContext _context;

    SimpleShaderLibrary _lib;

    std::vector<CommandBuffer*> _submittedBuffers;
    std::vector<ShaderLibrary*> _libraries;
    Pool<CommandBuffer, 1> _commandBufferPool;

    GLVaoCache _vaoCache;
    ResourceManager _resourceManager;
public:
    GLDevice();
    ~GLDevice();

    RenderDeviceApi GetDeviceApi();
    int32_t InitializeDevice(const DeviceInitialization& deviceInit) { return 0; }
    void ResizeWindow(uint32_t width, uint32_t height);
    void PrintDisplayAdapterInfo();

    BufferId AllocateBuffer(const BufferDesc& desc, const void* initialData = nullptr);

    ShaderId GetShader(ShaderType type, const std::string& functionName);
    void AddOrUpdateShaders(const std::vector<ShaderData>& shaderData);

    PipelineStateId CreatePipelineState(const PipelineStateDesc& desc);
    TextureId CreateTexture2D(PixelFormat format, uint32_t width, uint32_t height, void* data);
    TextureId CreateTextureArray(PixelFormat format, uint32_t levels, uint32_t width, uint32_t height, uint32_t depth);
    TextureId CreateTextureCube(PixelFormat format, uint32_t width, uint32_t height, void** data);
    VertexLayoutId CreateVertexLayout(const VertexLayoutDesc& desc);

    void DestroyResource(ResourceId resourceId);

    CommandBuffer* CreateCommandBuffer();
    void Submit(const std::vector<CommandBuffer*>& cmdBuffers);
    uint8_t* MapMemory(BufferId bufferId, BufferAccess access);
    void UnmapMemory(BufferId bufferId);
    void RenderFrame();

private:
    ShaderId CreateShader(const ShaderFunctionDesc& funcDesc, const ShaderData& data);
    ShaderId CreateShader(ShaderType type, const char* source);
    void BindResource(const Binding& binding);    

    size_t BuildKey(GLShaderProgram* vertexShader, GLBuffer* vertexBuffer, GLVertexLayout* vertexLayout);
    GLVertexArrayObject* GetOrCreateVertexArrayObject(GLShaderProgram* vertexShader, GLBuffer* vertexBuffer, const VertexStream& stream, GLBuffer* indexBuffer,
                                                      GLVertexLayout* vertexLayout);

    void Execute(CommandBuffer* cmdBuffer);
};
}
