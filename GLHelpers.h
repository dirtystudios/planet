//
// Created by Eugene Sturm on 4/6/15.
//

#ifndef __gl_helpers_h__
#define __gl_helpers_h__


#include <OpenGL/gl3.h>
#include "Log.h"
#include <unordered_map>
#include <fstream>
#include <cassert>

#ifndef NDEBUG
#   define GL_CHECK(func) \
do { \
func;\
GLenum err = glGetError(); \
if(err != GL_NO_ERROR) { fprintf(stderr, "GL error: %d (%s) %s\n", err , glEnumName(err), #func); assert(false); }  \
}  \
while (false)
#else
#   define GL_CHECK(func) func
#endif


#define STRINGIFY(x) #x
static const char* glEnumName(GLenum _enum) {
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
    }
    return "";
}


namespace gl {
    static GLuint CreateShaderFromFile(GLenum shader_type, const char* fpath) {
        std::ifstream fin(fpath);

        if(fin.fail()) {
            fprintf(stderr, "Failed to open file '%s'\n", fpath);
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
            fprintf(stderr, "Failed to compile shader (%s): %s\n", glEnumName(shader_type), log);        
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
            fprintf(stderr, "Failed to link program: %s\n", log);        
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
}


#endif
