#pragma once

#include "Helpers.h"
#include <array>

namespace gfx {
    
enum class RenderDeviceApi : uint8_t { Unknown = 0, OpenGL, Metal, D3D11, D3D12 };
RenderDeviceApi ApiFromString(const std::string& apiString);

}
