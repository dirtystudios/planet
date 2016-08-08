#pragma once

#include <string>
#include "ResourceTypes.h"
#include "ShaderType.h"

namespace gfx {
class ShaderLibrary {
public:
    virtual ShaderId GetShader(ShaderType type, const std::string& functionName) = 0;
    //    virtual const std::vector<ShaderFunctionDesc>& functions() const = 0;
};
}
