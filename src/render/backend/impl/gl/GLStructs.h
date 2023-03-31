#pragma once

#ifdef _WIN32
#include <GL/glew.h>
#else
#include <OpenGL/gl3.h>
#endif
#include <stdint.h>
#include <vector>
#include <string>
#include "GLTextureFormatDesc.h"
#include "Resource.h"
#include "GLUtils.h"
#include <algorithm>

namespace gfx {
struct GLUniformMetadata {
    std::string name;
    GLuint location;
    GLint size;
    GLenum type;
};
    
struct GLSamplerMetadata {
    GLUniformMetadata* uniform;
    int32_t slot{-1};
};

struct GLBlockUniformMetadata {
    std::string name;
    GLuint index;
    GLint size;
    GLint type;
    GLint offset;
    GLint arrayStride;
    GLint matrixStride;
};

struct GLUniformBlockMetadata {
    std::string name;
    GLint size;
    GLint index;
    uint32_t slot;
    std::vector<GLBlockUniformMetadata> uniforms;
};

struct GLAttributeMetadata {
    std::string name;
    GLenum type;
    GLint location;
    GLint size;
};

struct GLShaderMetadata {
    std::vector<GLUniformBlockMetadata> blocks;
    std::vector<GLAttributeMetadata> attributes;
    std::vector<GLUniformMetadata> uniforms;  
    std::vector<GLSamplerMetadata> samplers;
};

struct GLTexture : public Resource {
    ~GLTexture() {
        if(id) {
            GL_CHECK(glDeleteTextures(1, &id));
        }
    }
    
    GLuint id{0};
    GLenum type;
    GLTextureFormatDesc format;
    uint32_t width;
    uint32_t height;
};

struct GLPipelineState;
struct GLShaderProgram : public Resource {
    ~GLShaderProgram() {
        if(id) {
            for (GLPipelineState* pipeline : members) {
                // Note(eugene): We have pipelines referencing a shader that is
                // being destroy. How should this be handled?
            }
            GL_CHECK(glDeleteProgram(id));
        }
    }
    GLuint id{0};
    GLenum type;
    GLShaderMetadata metadata;
    std::vector<GLPipelineState*> members;
    
    GLint GetLocationForSamplerSlot(uint32_t slotIdx) {
        if(metadata.samplers.size() == 0) return -1;
        auto it = std::find_if(begin(metadata.samplers), end(metadata.samplers), [&slotIdx](const GLSamplerMetadata& sampler) { return sampler.slot == slotIdx; });
        return it != metadata.samplers.end() ? it->uniform->location : - 1;
    }
};

struct GLBlendState {
    bool enable{false};
    GLenum srcRgbFunc{GL_ONE};
    GLenum srcAlphaFunc{GL_ZERO};
    GLenum dstRgbFunc{GL_ONE};
    GLenum dstAlphaFunc{GL_ZERO};
    GLenum rgbMode{GL_FUNC_ADD};
    GLenum alphaMode{GL_FUNC_ADD};
};

struct GLRasterState {
    GLenum fillMode{GL_FILL};
    GLenum cullMode{GL_FALSE};
    GLenum windingOrder{GL_CCW};
};

struct GLDepthState {
    bool enable{true};
    GLenum depthWriteMask{GL_TRUE};
    GLenum depthFunc{GL_LESS};
};

struct GLVertexElement {
    GLenum type;
    uint32_t count;

    std::string ToString() const {
        return "GLVertexElement [type:" + std::to_string(type) + ", count:" + std::to_string(count) + "]";
    }
};

// Next time you think, "Why do I want vertex layout's again", it's because they will be needed
// to evenutally support multiple vertex data streams. Right now we only assume we will have one
// vertex buffer so it seems kind of pointless
struct GLVertexLayout : public Resource {
    std::vector<GLVertexElement> elements;
    size_t stride{0};
};

struct GLBuffer : public Resource {
    ~GLBuffer() {
        if(id)
            GL_CHECK(glDeleteBuffers(1, &id));
    }
    
    GLuint id{0};
    GLenum type;
    GLenum usage;
    GLVertexLayout* layout{nullptr};
};


struct GLPipelineState : public Resource {
    ~GLPipelineState() {
        if (vertexShader) {
            std::vector<GLPipelineState*>& shaderPipelineRefs = vertexShader->members;
            shaderPipelineRefs.erase(std::remove(shaderPipelineRefs.begin(), shaderPipelineRefs.end(), this),
                                     shaderPipelineRefs.end());
        }
        if (pixelShader) {
            std::vector<GLPipelineState*>& shaderPipelineRefs = pixelShader->members;
            shaderPipelineRefs.erase(std::remove(shaderPipelineRefs.begin(), shaderPipelineRefs.end(), this),
                                     shaderPipelineRefs.end());
        }

    }
    GLShaderProgram* vertexShader{nullptr};
    GLShaderProgram* pixelShader{nullptr};
    GLRasterState rasterState;
    GLBlendState blendState;
    GLDepthState depthState;
    GLVertexLayout* vertexLayout{nullptr};
    GLenum topology;
};

struct GLVertexArrayObject {
    GLuint id{0};
};
}
