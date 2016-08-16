#include "RenderDeviceApi.h"


namespace gfx {
RenderDeviceApi ApiFromString(const std::string& apiString) {
    static constexpr size_t d3d11StringCount                                = 1;
    static constexpr std::array<const char*, d3d11StringCount> d3d11Strings = {{"directx11"}};
    static constexpr size_t glStringCount                                   = 1;
    static constexpr std::array<const char*, glStringCount> glStrings       = {{"opengl"}};
    static constexpr size_t mtlStringCount                                  = 1;
    static constexpr std::array<const char*, mtlStringCount> mtlStrings     = {{"metal"}};
    
    std::string lowerCase = ToLowercase(apiString);
    
    for (const char* str : d3d11Strings) {
        if (lowerCase.compare(str) == 0) {
            return RenderDeviceApi::D3D11;
        }
    }
    
    for (const char* str : glStrings) {
        if (lowerCase.compare(str) == 0) {
            return RenderDeviceApi::OpenGL;
        }
    }
    
    for (const char* str : mtlStrings) {
        if (lowerCase.compare(str) == 0) {
            return RenderDeviceApi::Metal;
        }
    }
    
    return RenderDeviceApi::Unknown;
}
}
