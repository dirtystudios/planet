//
// Created by Eugene Sturm on 4/6/15.
//

#ifndef __gl_helpers_h__
#define __gl_helpers_h__

#ifdef _WIN32
#include <GL\glew.h>
#include <stdint.h>
#else
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>
#endif

#include "Log.h"
#include <unordered_map>
#include <fstream>
#include <cassert>
#include <vector>

#ifndef NDEBUG
#   define GL_CHECK(func) \
do { \
func;\
GLenum err = glGetError(); \
if(err != GL_NO_ERROR) { LOG_E("GL error: %d (%s) %s\n", err , GetGLenumString(err), #func); assert(false); }  \
}  \
while (false)
#else
#   define GL_CHECK(func) func
#endif


#define STRINGIFY(x) #x
static const char* GetGLenumString(GLenum _enum) {
#define GLENUM(_ty) case _ty: return #_ty

    switch (_enum)
    {
        GLENUM(GL_TEXTURE);
        GLENUM(GL_RENDERBUFFER);

        GLENUM(GL_INVALID_ENUM);
        GLENUM(GL_INVALID_VALUE);
        GLENUM(GL_INVALID_OPERATION);
        GLENUM(GL_OUT_OF_MEMORY);

        GLENUM(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);
        GLENUM(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT);
            //          GLENUM(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER);
            //          GLENUM(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER);
        GLENUM(GL_FRAMEBUFFER_UNSUPPORTED);
        GLENUM(GL_FLOAT_VEC2);
        GLENUM(GL_FLOAT_VEC3);
        GLENUM(GL_FLOAT_VEC4);
        GLENUM(GL_FLOAT_MAT2);
        GLENUM(GL_FLOAT_MAT3);
        GLENUM(GL_FLOAT_MAT4);
        GLENUM(GL_SAMPLER_2D);
        GLENUM(GL_SAMPLER_CUBE);
        GLENUM(GL_VERTEX_SHADER);
        GLENUM(GL_FRAGMENT_SHADER);
        GLENUM(GL_TESS_CONTROL_SHADER);
        GLENUM(GL_TESS_EVALUATION_SHADER);
        GLENUM(GL_FLOAT);
    }
    return "";
}


enum ParamType {
    Int32,
    Float,
    Float2,
    Float3,
    Float4,
    Float4x4,
    Count
};

static size_t SizeofParam(ParamType type) {
    switch(type) {
        case ParamType::Int32: {
            return sizeof(int32_t);
        }
        case ParamType::Float: {
            return sizeof(float);
        }
        case ParamType::Float2: {
            return sizeof(float) * 2;
        }
        case ParamType::Float3: {
            return sizeof(float) * 3;
        }
        case ParamType::Float4: {
            return sizeof(float) * 4;
        }
        case ParamType::Float4x4: {
            return sizeof(float) * 16;
        }
        default: {
            return 0;
        }
    }
}


struct VertexElement {
    ParamType type;
    VertexElement(ParamType type) : type(type) {};
};

struct VertexLayout {
    std::vector<VertexElement> elements;
    size_t stride { 0 };

    VertexLayout(std::vector<VertexElement> vert_elements) {
        for(const VertexElement &elem : vert_elements) {
            stride += SizeofParam(elem.type);
            elements.push_back(elem);
        }
    };

    VertexLayout() {};

    void Add(ParamType type) {
        elements.push_back(VertexElement(type));
        stride += SizeofParam(type);
    }
};

namespace gl {
    static GLuint CreateShaderFromFile(GLenum shader_type, const char* fpath) {
        std::ifstream fin(fpath);

        if(fin.fail()) {
            LOG_E("Failed to open file '%s'\n", fpath);
            assert(false);
        }

        std::string ss((std::istreambuf_iterator<char>(fin)),
                        std::istreambuf_iterator<char>());

        const char* source = ss.c_str();
        GLuint shader_id = glCreateShader(shader_type);        

        GL_CHECK(glShaderSource(shader_id, 1, (const GLchar**)&source, NULL));
        GL_CHECK(glCompileShader(shader_id));
                    
        GLint compiled = 0;
        GL_CHECK(glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled) );
        
        if (!compiled) {        
            GLsizei len;
            char log[1024];        
            GL_CHECK(glGetShaderInfoLog(shader_id, sizeof(log), &len, log));
            LOG_E("Failed to compile shader (%s): %s\n", GetGLenumString(shader_type), log);        
            assert(false);
        } 

        fin.close();

        return shader_id;
    }

    static GLuint CreateProgram(GLuint* shaders, uint32_t len) {

        GLuint program_id = glCreateProgram();
        for(uint32_t idx = 0; idx < len; ++idx) {
            GLuint shader_id = shaders[idx];            
            GL_CHECK(glAttachShader(program_id, shader_id));
        }

        GL_CHECK(glLinkProgram(program_id));

        GLint linked = 0;
        GL_CHECK(glGetProgramiv(program_id, GL_LINK_STATUS, &linked));        
        if(!linked) {
            char log[1024];
            GL_CHECK(glGetProgramInfoLog(program_id, 1024, nullptr, log));
            LOG_E("Failed to link program: %s\n", log);        
            assert(false);
        }

        return program_id;
    }

    static GLuint CreateBuffer(GLenum buffer_type, void* data, size_t size, GLenum usage) {                 
        GLuint buffer_id = 0;
        GL_CHECK(glGenBuffers(1, &buffer_id));                
        GL_CHECK(glBindBuffer(buffer_type, buffer_id));
        GL_CHECK(glBufferData(buffer_type, size, data, usage));            
        return buffer_id;
    }

    static void UpdateUniformBuffer(void* data, size_t size) {
        void* stuff = (void*)glMapBufferRange(GL_UNIFORM_BUFFER, 0, size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        memcpy(stuff, data, size);
        glUnmapBuffer(GL_UNIFORM_BUFFER);
    }   


   static GLuint CreateTexture2D(GLenum tex_format, GLenum data_format, GLenum data_type, uint32_t width, uint32_t height, void* data) {
        GLuint tex_id = 0;
        GL_CHECK(glGenTextures(1, &tex_id));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, tex_id));
        GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, tex_format, width, height, 0, data_format, data_type, data));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        return tex_id;       
   }
   
   
    static GLuint CreateTexture2DArray(GLenum tex_format, uint32_t levels, uint32_t width, uint32_t height, uint32_t depth) {   
        GLuint tex_id = 0;
        GL_CHECK(glGenTextures(1, &tex_id));
        GL_CHECK(glBindTexture(GL_TEXTURE_2D_ARRAY, tex_id));
        GL_CHECK(glTexStorage3D(GL_TEXTURE_2D_ARRAY, levels, tex_format, width, height, depth));
        glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        return tex_id;
    } 

    static void UpdateTexture2DArray(GLuint tex_id, GLenum data_format, GLenum data_type, uint32_t width, uint32_t height, uint32_t array_index, void* data) {
        GL_CHECK(glBindTexture(GL_TEXTURE_2D_ARRAY, tex_id));
        uint32_t xoffset = 0, yoffset = 0, level = 0, depth = 1;
        GL_CHECK(glTexSubImage3D(GL_TEXTURE_2D_ARRAY, level, xoffset, yoffset, array_index, width, height, depth, data_format, data_type, data));                
    }

    static GLint GetUniformLocation(GLuint program, const char* name) {
        GLint loc = glGetUniformLocation(program, name);
        assert(loc >= 0);
        return loc;
    }

    static void SetUniform(GLuint program, const char* name, ParamType param_type, void* data) {
        GLint location = GetUniformLocation(program, name);
        assert(location >= 0);

       switch(param_type) {
            case ParamType::Float: {
                GL_CHECK(glUniform1f(location, (float)*((float*)data)));
                break;
            }
            case ParamType::Int32: {
                GL_CHECK(glUniform1i(location, (int)*((int*)data)));
                break;
            }
            case ParamType::Float4x4: {
                GL_CHECK(glUniformMatrix4fv(location, 1, GL_FALSE, (float*)data));
                break;
            }
            default: {
                LOG_E("gfx", "Unsupported uniform type");
                assert(false);
            }
        }
    }

    static GLuint CreateVertexArrayObject(GLuint vertex_buffer_id, const VertexLayout& layout) {
        GLuint vao_id = 0;
        GL_CHECK(glGenVertexArrays(1, &vao_id));
        GL_CHECK(glBindVertexArray(vao_id));
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id));
        
        size_t offset = 0;
        for(uint32_t idx = 0; idx < layout.elements.size(); ++idx) {            
            const VertexElement* vert_elem = &layout.elements[idx];
            
            GLenum type = GL_FALSE;
            GLuint size = 0;
            switch(vert_elem->type) {
                case ParamType::Float:
                    type = GL_FLOAT;
                    size = 1;
                    break;
                case ParamType::Float2:
                    type = GL_FLOAT;
                    size = 2;
                    break;
                case ParamType::Float3:
                    type = GL_FLOAT;
                    size = 3;
                    break;
                case ParamType::Float4:
                    type = GL_FLOAT;
                    size = 4;
                    break;
                default:
                    LOG_E("%s", "Unknown vertex element type");
                    return false;
            }
            
            LOG_D("VertexArrayAttribute size:%d type:%s offset:%d to location:%d stride:%d", size, GetGLenumString(type), offset, idx, layout.stride);
            GL_CHECK(glEnableVertexAttribArray(idx));
            GL_CHECK(glVertexAttribPointer(idx, size, type, GL_FALSE, layout.stride, (const void*)offset));
            offset += SizeofParam(vert_elem->type);
        }                
        return vao_id;
    }
}


#endif
