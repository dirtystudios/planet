#pragma once

#include "RenderDevice.h"
#include "GLStructs.h"
#include "GLContext.h"

#include <unordered_map>
#include "Frame.h"

namespace graphics {
class GLDevice : public RenderDevice {
private:
    using GLVaoCacheKey = size_t;
    using GLVaoCache    = std::unordered_map<GLVaoCacheKey, GLVertexArrayObject*>;

private:
    GLContext _context;

    Frame* _currentFrame;    

    std::unordered_map<uint32_t, GLBuffer*> _buffers;
    std::unordered_map<uint32_t, GLShaderProgram*> _shaders;
    std::unordered_map<uint32_t, GLTexture*> _textures;
    std::unordered_map<uint32_t, GLShaderParameter*> _shaderParams;
    std::unordered_map<uint32_t, GLPipelineState*> _pipelineStates;
    std::unordered_map<uint32_t, GLVertexLayout*> _vertexLayouts;

    GLVaoCache _vaoCache;
public:
    GLDevice();
    virtual ~GLDevice(){};

    int32_t InitializeDevice(const DeviceInitialization& deviceInit) { return 0; }
    void ResizeWindow(uint32_t width, uint32_t height);
    void PrintDisplayAdapterInfo();

    BufferId CreateBuffer(BufferType type, void* data, size_t size, BufferUsage usage);
    ShaderId CreateShader(ShaderType type, const std::string& source);
    ShaderParamId CreateShaderParam(ShaderId shader, const char* param, ParamType paramType);
    PipelineStateId CreatePipelineState(const PipelineStateDesc& desc);
    TextureId CreateTexture2D(TextureFormat format, uint32_t width, uint32_t height, void* data);
    TextureId CreateTextureArray(TextureFormat format, uint32_t levels, uint32_t width, uint32_t height,
                                 uint32_t depth);
    TextureId CreateTextureCube(TextureFormat format, uint32_t width, uint32_t height, void** data);
    VertexLayoutId CreateVertexLayout(const VertexLayoutDesc& desc);

    Frame* BeginFrame();
    void SubmitFrame();

    void DestroyBuffer(BufferId buffer);
    void DestroyShader(ShaderId shader);
    void DestroyShaderParam(ShaderParamId shaderParam);
    void DestroyPipelineState(PipelineStateId pipelineState);
    void DestroyTexture(TextureId texture);
    void DestroyVertexLayout(VertexLayoutId layout);

private:
    uint32_t GenerateId();

    size_t BuildKey(GLShaderProgram* vertexShader, GLBuffer* vertexBuffer, GLVertexLayout* vertexLayout);
    GLVertexArrayObject* GetOrCreateVertexArrayObject(GLShaderProgram* vertexShader, GLBuffer* vertexBuffer, GLBuffer* indexBuffer,
                                                            GLVertexLayout* vertexLayout);

    void Execute(const std::vector<DrawTask*>& drawTask);
    void Execute(const std::vector<BufferUpdate*>& bufferUpdate);
    void Execute(const std::vector<TextureUpdate*>& textureUpdate);
    void Execute(const std::vector<TextureBind*>& textureUpdate);
    void Execute(const std::vector<ShaderParamUpdate*>& shaderParamUpdate);

    template <class T>
    bool DestroyResource(uint32_t handle, std::unordered_map<uint32_t, T*>& map, std::function<void(T*)> destroy) {
        auto it = map.find(handle);
        if (it == map.end()) {
            return false;
        }

        T* resource = (*it).second;

        if (resource->id) {
            destroy(resource);
        } else {
            return false;
        }
        map.erase(it);
        return true;
    }

    template <class T> T* GetResource(std::unordered_map<uint32_t, T*>& map, uint32_t handle) {
        auto it = map.find(handle);
        if (it == map.end()) {
            return nullptr;
        }

        return (*it).second;
    }
};
}