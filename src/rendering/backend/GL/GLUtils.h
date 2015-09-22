//
// Created by Eugene Sturm on 4/6/15.
//

#ifndef DG_GLUTILS_H
#define DG_GLUTILS_H

#ifdef _WIN32
    #include "GL/glew.h"
#else
    #include <OpenGL/gl3.h>
#endif

#include "Log.h"

#ifndef NDEBUG
#   define GL_CHECK(func) \
do { \
func;\
GLenum err = glGetError(); \
if(err != GL_NO_ERROR) { LOG_E("GL error: %d (%s) %s\n", err , glEnumName(err), #func); assert(false); }  \
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
            //			GLENUM(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER);
            //			GLENUM(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER);
        GLENUM(GL_FRAMEBUFFER_UNSUPPORTED);
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
        GLENUM(GL_SAMPLER_CUBE);
    }
    return "UNKNOWN GLENUM";
}

template <class T>
static void hash_combine(std::size_t& seed, const T& v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}



#endif //DG_GLUTILS_H
