#include "GLContext.h"
#include <cassert>

using namespace graphics;

GLContext::GLContext() {
    GL_CHECK(glGenProgramPipelines(1, &_programPipeline));
    GL_CHECK(glBindProgramPipeline(_programPipeline));

    SetClearColor(0.1f, 0.1f, 0.1f, 1.f);

    // glDepthFunc(GL_LESS);
    //    glEnable(GL_DEPTH_TEST);
       glEnable(GL_CULL_FACE);
       glFrontFace(GL_CCW);
       glCullFace(GL_BACK);
}

GLContext::~GLContext() {
    // cleanup pipeline
}

void GLContext::WriteBufferData(GLBuffer* buffer, void* data, size_t size) {
    BindBuffer(buffer);
    GL_CHECK(glBufferData(buffer->type, size, data, buffer->usage));
}

void GLContext::WriteTextureData(GLTexture* texture, void* data, size_t size) { assert(false); }

void GLContext::BindVertexArrayObject(GLVertexArrayObject* vao) {
    assert(vao);
    if (!_activeVao || _activeVao != vao) {
        GL_CHECK(glBindVertexArray(vao->id));
        _activeVao = vao;
    }
}

void GLContext::ForceBindBuffer(GLBuffer* buffer) { BindBuffer(buffer, true); }

void GLContext::BindBuffer(GLBuffer* buffer, bool force) {
    assert(buffer);
    BufferType bufferType   = GLEnumAdapter::ConvertBufferType(buffer->type);
    GLBuffer** activeBuffer = &_activeBuffers[static_cast<size_t>(bufferType)];
    if (force || !*activeBuffer || *activeBuffer != buffer) {
        GL_CHECK(glBindBuffer(buffer->type, buffer->id));
        *activeBuffer = buffer;
    }
}

void GLContext::BindTexture(TextureSlot slot, GLTexture* texture) {
    // do we care if same texture is bound to multiple slots?
    GLTexture* activeTexture = _activeTextures[static_cast<uint8_t>(slot)];
    if (!activeTexture || activeTexture != texture) {
        GL_CHECK(glBindTexture(texture->type, texture->id));
    }
}

void GLContext::BindTextureAsShaderResource(ShaderStage stage, TextureSlot slot, GLTexture* texture) {
    uint32_t slotIdx = static_cast<uint32_t>(slot);
    GL_CHECK(glActiveTexture(GL_TEXTURE0 + slotIdx));
    BindTexture(slot, texture); 

    GLShaderProgram* activeShader = _activeShaders[static_cast<size_t>(stage)];    
    assert(activeShader);

    GLint location = -1;
    switch(slot) {
        case TextureSlot::Base: {            
            location = glGetUniformLocation(activeShader->id, "baseTexture");
            break;
        }
        default: assert(false);
    }
    
    GL_CHECK();
    assert(location >= 0);

    GL_CHECK(glProgramUniform1i(activeShader->id, location, slotIdx));
}

void GLContext::BindPipelineState(GLPipelineState* pipelineState) {
    if (pipelineState == _activePipelineState) {
        return;
    }

    // TODO: how to we handle one of these steps failing. do we rollback? move on?
    ResolveRasterState((_activePipelineState ? &_activePipelineState->rasterState : nullptr),
                       pipelineState->rasterState);
    ResolveBlendState((_activePipelineState ? &_activePipelineState->blendState : nullptr), pipelineState->blendState);
    ResolveDepthState((_activePipelineState ? &_activePipelineState->depthState : nullptr), pipelineState->depthState);
    BindShader(pipelineState->vertexShader);
    BindShader(pipelineState->pixelShader);

    _activePipelineState = pipelineState;
}

void GLContext::BindShader(GLShaderProgram* shader) {
    ShaderType shaderType          = GLEnumAdapter::ConvertShaderType(shader->type);
    GLShaderProgram** activeShader = &_activeShaders[static_cast<uint8_t>(shaderType)];
    if (!(*activeShader) || (*activeShader) != shader) {
        GLenum bit = GL_FALSE;
        switch (shaderType) {
        case ShaderType::VertexShader:
            bit = GL_VERTEX_SHADER_BIT;
            break;
        case ShaderType::PixelShader:
            bit = GL_FRAGMENT_SHADER_BIT;
            break;
        default:
            assert(false);
        }

        GL_CHECK(glUseProgramStages(_programPipeline, bit, shader->id));
        *activeShader = shader;
    }
}

void GLContext::WriteShaderParamater(GLShaderParameter* shaderParam, void* data, size_t size) {
    GLShaderProgram* program = shaderParam->program;
    GLint location           = shaderParam->metadata.location;
    ParamType paramType      = GLEnumAdapter::ConvertParamType(shaderParam->metadata.type);

    assert(size == GetByteCount(paramType));
    assert(IsBound(program));
    GL_CHECK();
    switch (paramType) {
    case ParamType::Float: {
        GL_CHECK(glProgramUniform1f(program->id, location, (float)*((float*)data)));
        break;
    }
    case ParamType::Int32: {
        GL_CHECK(glProgramUniform1i(program->id, location, (int)*((int*)data)));
        break;
    }
    case ParamType::Float4x4: {
        GL_CHECK(glProgramUniformMatrix4fv(program->id, location, 1, GL_FALSE, (float*)data));
        break;
    }
    case ParamType::Float3: {
        GL_CHECK(glProgramUniform3fv(program->id, location, 1, (float*)data));
        break;
    }
    case ParamType::Float2: {
        GL_CHECK(glProgramUniform2fv(program->id, location, 1, (float*)data));
        break;
    }
    case ParamType::Float4: {
        GL_CHECK(glProgramUniform4fv(program->id, location, 1, (float*)data));
        break;
    }
    default: { assert(false); }
    }
}

bool GLContext::IsBound(GLShaderProgram* shader) {
    ShaderType shaderType = GLEnumAdapter::ConvertShaderType(shader->type);
    return _activeShaders[static_cast<uint8_t>(shaderType)] == shader;
}

void GLContext::SetClearColor(float r, float g, float b, float a) {
    if (_activeClearColor[0] == r && _activeClearColor[1] == g && _activeClearColor[2] == b &&
        _activeClearColor[3] == a) {
        return;
    }

    _activeClearColor[0] = r;
    _activeClearColor[1] = g;
    _activeClearColor[2] = b;
    _activeClearColor[3] = a;
    GL_CHECK(glClearColor(_activeClearColor[0], _activeClearColor[1], _activeClearColor[2], _activeClearColor[3]));
}

void GLContext::SetClearDepth(float d) {
    if (_activeClearDepth == d) {
        return;
    }

    _activeClearDepth = d;
    GL_CHECK(glClearDepth(_activeClearDepth));
}

void GLContext::BindRasterState(const GLRasterState& to) {    
    if (to.cullMode == GL_FALSE) {
        GL_CHECK(glDisable(GL_CULL_FACE));
    } else {
        GL_CHECK(glEnable(GL_CULL_FACE));
        GL_CHECK(glCullFace(to.cullMode));
    }

    GL_CHECK(glFrontFace(to.windingOrder));
    GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, to.fillMode));
}

void GLContext::ResolveRasterState(const GLRasterState* from, const GLRasterState& to) {    
    if (!from) {
        BindRasterState(to);
        return;
    }

    if (from->cullMode != to.cullMode) {
        if (from->cullMode == GL_FALSE || to.cullMode != GL_FALSE) {
            GL_CHECK(glEnable(GL_CULL_FACE));
            GL_CHECK(glCullFace(to.cullMode));
        } else if (from->cullMode != GL_FALSE || to.cullMode == GL_FALSE) {
            GL_CHECK(glDisable(GL_CULL_FACE));
        }
    }

    if (from->windingOrder != to.windingOrder) {
        GL_CHECK(glFrontFace(to.windingOrder));
    }

    if (from->fillMode != to.fillMode) {        
        GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, to.fillMode));
    }
}

void GLContext::BindDepthState(const GLDepthState& to) {
    if (to.enable) {
        GL_CHECK(glEnable(GL_DEPTH_TEST));
    } else {
        GL_CHECK(glDisable(GL_DEPTH_TEST));
    }

    GL_CHECK(glDepthMask(to.depthWriteMask));
    GL_CHECK(glDepthFunc(to.depthFunc));
}

void GLContext::ResolveDepthState(const GLDepthState* from, const GLDepthState& to) {
    if (!from) {
        BindDepthState(to);
        return;
    }

    if (from->enable != to.enable) {
        if (to.enable) {
            GL_CHECK(glEnable(GL_DEPTH_TEST));
        } else {
            GL_CHECK(glDisable(GL_DEPTH_TEST));
        }
    }

    if (from->depthWriteMask != to.depthWriteMask) {
        GL_CHECK(glDepthMask(to.depthWriteMask));
    }

    if (from->depthFunc != to.depthFunc) {
        GL_CHECK(glDepthFunc(to.depthFunc));
    }
}

void GLContext::BindBlendState(const GLBlendState& to) {
    if (to.enable) {
        GL_CHECK(glEnable(GL_BLEND));
    } else {
        GL_CHECK(glDisable(GL_BLEND));
    }

    GL_CHECK(glBlendEquationSeparate(to.rgbMode, to.alphaMode));
    GL_CHECK(glBlendFuncSeparate(to.srcRgbFunc, to.dstRgbFunc, to.srcAlphaFunc, to.dstAlphaFunc));
}

void GLContext::ResolveBlendState(const GLBlendState* from, const GLBlendState& to) {
    if (!from) {
        BindBlendState(to);
        return;
    }

    size_t fromBlendEq = GLEnumAdapter::Hash({from->rgbMode, from->alphaMode});
    size_t toBlendEq = GLEnumAdapter::Hash({to.rgbMode, to.alphaMode});
    size_t fromBlendFuncHash =
        GLEnumAdapter::Hash({from->srcRgbFunc, from->dstRgbFunc, from->srcAlphaFunc, from->dstAlphaFunc});
    size_t toBlendFuncHash = GLEnumAdapter::Hash({to.srcRgbFunc, to.dstRgbFunc, to.srcAlphaFunc, to.dstAlphaFunc});

    if (from->enable != to.enable) {
        if (to.enable) {
            GL_CHECK(glEnable(GL_BLEND));
        } else {
            GL_CHECK(glDisable(GL_BLEND));
        }
    }

    if (fromBlendEq != toBlendEq) {
        GL_CHECK(glBlendEquationSeparate(to.rgbMode, to.alphaMode));
    }

    if (fromBlendFuncHash != toBlendFuncHash) {
        GL_CHECK(glBlendFuncSeparate(to.srcRgbFunc, to.dstRgbFunc, to.srcAlphaFunc, to.dstAlphaFunc));
    }
}