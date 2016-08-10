#pragma once


#include "RenderDevice.h"

namespace gfx {
class SimpleShaderLibrary : public ShaderLibrary {
private:
    std::vector<std::pair<ShaderFunctionDesc, ShaderId>> _shaders;
public:
    void AddShader(ShaderId shaderId, const ShaderFunctionDesc& desc);
    ShaderId GetShader(ShaderType type, const std::string& functionName);
};
}
