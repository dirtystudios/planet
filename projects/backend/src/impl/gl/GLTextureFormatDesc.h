#pragma once
#ifdef _WIN32
#include <GL/glew.h>
#else
#include <OpenGL/OpenGL.h>
#endif

namespace gfx {
struct GLTextureFormatDesc {
    GLenum internalFormat;
    GLenum dataType;    
    GLenum dataFormat;
};
}
