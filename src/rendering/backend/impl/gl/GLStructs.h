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

using namespace std;
namespace graphics {
struct GLUniformMetadata {
    string name;
    GLuint location;
    GLint size;
    GLenum type;
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
    vector<GLBlockUniformMetadata> uniforms;
};

struct GLAttributeMetadata {
    string name;
    GLenum type;
    GLint location;
    GLint size;

    std::string ToString() const {
        return "GLAttributeMetadata [name:'" + name + "', type:" + std::to_string(type) + ", location:" +
               std::to_string(location) + " size:" + std::to_string(size) + "]";
    }
};

struct GLShaderMetadata {
    vector<GLUniformBlockMetadata> blocks;
    vector<GLAttributeMetadata> attributes;
    vector<GLUniformMetadata> uniforms;
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
    bool enable{false};
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