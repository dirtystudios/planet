#pragma once

#include "Log.h"

#ifdef _WIN32
#include <GL/glew.h>
#else
#include <OpenGL/OpenGL.h>
#endif

static std::string GLEnumToString(GLenum glEnum) {
#define STRINGIFY(x) #x
#define GLENUM(_ty)                                                                                                    \
case _ty:                                                                                                          \
return #_ty
    
    switch (glEnum) {
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
            GLENUM(GL_FLOAT);
            GLENUM(GL_FLOAT_VEC2);
            GLENUM(GL_FLOAT_VEC3);
            GLENUM(GL_FLOAT_VEC4);
            GLENUM(GL_FLOAT_MAT2);
            GLENUM(GL_FLOAT_MAT3);
            GLENUM(GL_FLOAT_MAT4);
            GLENUM(GL_INT);
            GLENUM(GL_INT_VEC2);
            GLENUM(GL_INT_VEC3);
            GLENUM(GL_INT_VEC4);
            GLENUM(GL_UNSIGNED_INT);
            GLENUM(GL_UNSIGNED_INT_VEC2);
            GLENUM(GL_UNSIGNED_INT_VEC3);
            GLENUM(GL_SAMPLER_2D);
            GLENUM(GL_SAMPLER_2D_ARRAY);
            GLENUM(GL_SAMPLER_CUBE);
            GLENUM(GL_RGBA);
            GLENUM(GL_RGB);
            GLENUM(GL_UNSIGNED_BYTE);
    }
    return "UNKNOWN GLENUM";
}


#ifndef NDEBUG
#define GL_CHECK(func)                                                                                                 \
do {                                                                                                               \
func;                                                                                                          \
GLenum err = glGetError();                                                                                     \
if (err != GL_NO_ERROR) {                                                                                      \
LOG_E("GL error: %d (%s) %s\n", err, GLEnumToString(err).c_str(), #func);                          \
assert(false);                                                                                             \
}                                                                                                              \
} while (false)
#else
#define GL_CHECK(func) func
#endif
