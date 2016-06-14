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
#include <algorithm>

using namespace std;
namespace graphics {
struct GLUniformMetadata {
    string name;
    GLuint location;
    GLint size;
    GLenum type;
};
    
struct GLSamplerMetadata {
    GLUniformMetadata* uniform;
    int32_t slot{-1};
};

struct GLBlockUniformMetadata {
    string name;
    GLuint index;
    GLint size;
    GLint type;
    GLint offset;
    GLint arrayStride;
    GLint matrixStride;
};

struct GLUniformBlockMetadata {
    string name;
    GLint size;
    GLint index;
    uint32_t slot;
    vector<GLBlockUniformMetadata> uniforms;
};

struct GLAttributeMetadata {
    string name;
    GLenum type;
    GLint location;
    GLint size;
};

struct GLShaderMetadata {
    vector<GLUniformBlockMetadata> blocks;
    vector<GLAttributeMetadata> attributes;
    vector<GLUniformMetadata> uniforms;  
    vector<GLSamplerMetadata> samplers;
};

struct GLTexture {
    GLuint id;
    GLenum type;
    GLTextureFormatDesc format;
};

class GLResource {
public:
    size_t GetId();
};

struct GLPipelineState;
struct GLShaderProgram {
    GLuint id;
    GLenum type;
    GLShaderMetadata metadata;
    vector<GLPipelineState*> members;
    
    GLint GetLocationForSamplerSlot(uint32_t slotIdx) {
        if(metadata.samplers.size() == 0) return -1;
        auto it = std::find_if(begin(metadata.samplers), end(metadata.samplers), [&slotIdx](const GLSamplerMetadata& sampler) { return sampler.slot == slotIdx; });
        return it != metadata.samplers.end() ? it->uniform->location : - 1;
    }
};

struct GLShaderParameter {
    GLShaderProgram* program;
    GLUniformMetadata metadata;
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
struct GLVertexLayout {
    std::vector<GLVertexElement> elements;
    size_t stride{0};
};

struct GLBuffer {
    GLuint id;
    GLenum type;
    GLenum usage;
    GLVertexLayout* layout{nullptr};
};

struct GLPipelineState {
    GLShaderProgram* vertexShader;
    GLShaderProgram* pixelShader;
    GLRasterState rasterState;
    GLBlendState blendState;
    GLDepthState depthState;
    GLVertexLayout* vertexLayout;
    GLenum topology;
};

struct GLVertexArrayObject {
    GLuint id{0};
};
}