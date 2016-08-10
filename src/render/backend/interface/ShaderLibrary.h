#pragma once

#include <string>
#include "ShaderType.h"
#include "ResourceTypes.h"

namespace gfx {
class ShaderLibrary {
public:
    virtual ShaderId GetShader(ShaderType type, const std::string& functionName) = 0;
};
}
