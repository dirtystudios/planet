#pragma once

#include "TextureSlot.h"
#include "ShaderType.h"
#include "TextureSlot.h"
#include "BufferAccess.h"
#include "GLEnumAdapter.h"
#include "GLStructs.h"
#include <array>
#ifdef _WIN32
#include <GL/glew.h>
#else
#include <OpenGL/OpenGL.h>
#endif
#include "BufferType.h"
#include "Helpers.h"
#include "ShaderStage.h"
#ifndef NDEBUG
#define GL_CHECK(func)                                                                                                 \
    do {                                                                                                               \
        func;                                                                                                          \
        GLenum err = glGetError();                                                                                     \
        if (err != GL_NO_ERROR) {                                                                                      \
            LOG_E("GL error: %d (%s) %s\n", err, GLEnumAdapter::Convert(err).c_str(), #func);                          \
            assert(false);                                                                                             \
        }                                                                                                              \
    } while (false)                                                                                               
#else
#define GL_CHECK(func) func
#endif
namespace gfx {

class GLContext {
private:
    static constexpr size_t kMaxSupportedSlots = 8;
    
    // Todo: might be faster to use ids
    std::array<GLTexture*, static_cast<uint8_t>(TextureSlot::Count)>        _activeTextures {};
    std::array<GLBuffer*, static_cast<uint8_t>(BufferType::Count)>          _activeBuffers {};
    std::array<GLShaderProgram*, static_cast<uint8_t>(ShaderType::Count)>   _activeShaders {};
    std::array<GLBuffer*, kMaxSupportedSlots>                               _constantBufferSlots {};
    
    GLPipelineState* _activePipelineState{nullptr};
    GLVertexArrayObject* _activeVao{nullptr};
    
    float _activeClearColor[4];
    float _activeClearDepth;

    GLuint _programPipeline{0};

public:
    GLContext();
    ~GLContext();

    void WriteBufferData(GLBuffer* buffer, void* data, size_t size);
    void WriteTextureData(GLTexture* texture, void* data, size_t size);
    void ForceBindBuffer(GLBuffer* buffer);
    void BindBuffer(GLBuffer* buffer, bool force = false);
    void BindTexture(uint32_t slot, GLTexture* texture);
    void BindTextureAsShaderResource(GLTexture* texture, uint32_t slot);
    void BindPipelineState(GLPipelineState* pipelineState);
    void BindShader(GLShaderProgram* shader);
    void BindVertexArrayObject(GLVertexArrayObject* vao);
    void BindUniformBufferToSlot(GLBuffer* cbuffer, uint32_t slot);
    void SetClearColor(float r, float g, float b, float a);
    void SetClearDepth(float d);
    void SetDepthState(const DepthState& depthState);
    void SetRasterState(const RasterState& rasterState);
    void SetBlendState(const BlendState& blendState);
    void WriteShaderParamater(GLShaderParameter* shaderParam, void* data, size_t size);
    bool IsBound(GLShaderProgram* shader);
    uint8_t* Map(GLBuffer* buffer, BufferAccess access);
    void Unmap(GLBuffer* buffer);

private:
    void BindDepthState(const GLDepthState& to);
    void BindBlendState(const GLBlendState& to);
    void BindRasterState(const GLRasterState& to);
    void ResolveRasterState(const GLRasterState* from, const GLRasterState& to);
    void ResolveDepthState(const GLDepthState* from, const GLDepthState& to);
    void ResolveBlendState(const GLBlendState* from, const GLBlendState& to);
    GLint GetUniformLocation(GLShaderProgram* program, const char* name);
};
}
