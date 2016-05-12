#pragma once
#ifdef _WIN32
#include <GL/glew.h>
#else
#include <OpenGL/OpenGL.h>
#endif

namespace graphics {
struct GLTextureFormatDesc {
    GLenum internalFormat;
    GLenum dataType;    
    GLenum dataFormat;
};
}