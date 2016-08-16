#pragma once

#include <stdint.h>
#include <functional>
#include <unordered_map>
#include <string>

namespace gfx {
enum class ShaderType : uint8_t {
    VertexShader = 0,
    TessControlShader,
    TessEvalShader,
    PixelShader,
    Count,
};
}

static std::string ToString(gfx::ShaderType type) {
    static const std::unordered_map<gfx::ShaderType, std::string> ShaderTypeStringMap = {
        {gfx::ShaderType::VertexShader, "VertexShader"},
        {gfx::ShaderType::TessControlShader, "TessControlShader"},
        {gfx::ShaderType::TessEvalShader, "TessEvalShader"},
        {gfx::ShaderType::PixelShader, "PixelShader"}};
    return ShaderTypeStringMap.find(type)->second;
}
