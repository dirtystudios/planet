#pragma once

#include "ShaderType.h"
#include <sstream>
#include <string>

namespace gfx {
struct ShaderFunctionDesc {
    ShaderType type{ShaderType::VertexShader};
    std::string functionName;
    std::string entryPoint;
};
}

static std::string ToString(const gfx::ShaderFunctionDesc& f) {
    std::stringstream ss;
    ss << "ShaderFunctionDesc: {"
       << "type:" << ToString(f.type) << ", "
       << "functionName:" << f.functionName << ", "
       << "entryPoint:" << f.entryPoint << "}";

    return ss.str();
}
