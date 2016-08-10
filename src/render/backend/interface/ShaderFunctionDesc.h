#pragma once

#include "ShaderType.h"
#include <string>

namespace gfx {
struct ShaderFunctionDesc {
    ShaderType type{ShaderType::VertexShader};
    std::string functionName;
    std::string entryPoint;
};
}
