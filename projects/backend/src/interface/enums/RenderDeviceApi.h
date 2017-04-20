#pragma once

#include <array>

namespace gfx {
    
enum class RenderDeviceApi : uint8_t { Unknown = 0, OpenGL, Metal, D3D11 };
RenderDeviceApi ApiFromString(const std::string& apiString);

}
