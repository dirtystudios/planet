#include "SimpleShaderLibrary.h"

namespace gfx {
void SimpleShaderLibrary::AddShader(ShaderId shaderId, const ShaderFunctionDesc& desc) {
    _shaders.emplace_back(desc, shaderId);
}

ShaderId SimpleShaderLibrary::GetShader(ShaderType type, const std::string& functionName) {
    auto shader = std::find_if(begin(_shaders), end(_shaders), [&](const std::pair<ShaderFunctionDesc, ShaderId>& p) {
        return p.first.type == type && p.first.functionName == functionName;
    });
    
    return shader != _shaders.end() ? shader->second : 0;
}
}
