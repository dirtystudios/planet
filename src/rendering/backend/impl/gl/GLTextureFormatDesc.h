#pragma once
#include <GL/glew.h>

namespace graphics {
struct GLTextureFormatDesc {
    GLenum internalFormat;
    GLenum dataType;    
    GLenum dataFormat;
};
}