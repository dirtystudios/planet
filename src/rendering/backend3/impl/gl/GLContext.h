#pragma once

#include "TextureSlot.h"
#include "ShaderType.h"
#include "TextureSlot.h"
#include "GLEnumAdapter.h"
#include "GLStructs.h"
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
namespace graphics {

class GLContext {
private:
    GLTexture* _activeTextures[static_cast<uint8_t>(TextureSlot::Count)];
    GLBuffer* _activeBuffers[static_cast<uint8_t>(BufferType::Count)];
    GLShaderProgram* _activeShaders[static_cast<uint8_t>(ShaderType::Count)];
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
    void BindTexture(TextureSlot slot, GLTexture* texture);
    void BindTextureAsShaderResource(ShaderStage stage, TextureSlot slot, GLTexture* texture);
    void BindPipelineState(GLPipelineState* pipelineState);
    void BindShader(GLShaderProgram* shader);
    void BindVertexArrayObject(GLVertexArrayObject* vao);

    void SetClearColor(float r, float g, float b, float a);
    void SetClearDepth(float d);
    void SetDepthState(const DepthState& depthState);
    void SetRasterState(const RasterState& rasterState);
    void SetBlendState(const BlendState& blendState);

    void WriteShaderParamater(GLShaderParameter* shaderParam, void* data, size_t size);
    bool IsBound(GLShaderProgram* shader);

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
