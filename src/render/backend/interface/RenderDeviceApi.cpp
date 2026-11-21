#include "RenderDeviceApi.h"


namespace gfx {
RenderDeviceApi ApiFromString(const std::string& apiString) {   
    std::string lowerCase = ToLowercase(apiString);
    
    for (const char* str : { "directx11" }) {
        if (lowerCase.compare(str) == 0) {
            return RenderDeviceApi::D3D11;
        }
    }
    
    for (const char* str : { "opengl" }) {
        if (lowerCase.compare(str) == 0) {
            return RenderDeviceApi::OpenGL;
        }
    }
    
    for (const char* str : { "metal" }) {
        if (lowerCase.compare(str) == 0) {
            return RenderDeviceApi::Metal;
        }
    }
    
    return RenderDeviceApi::Unknown;
}
}
