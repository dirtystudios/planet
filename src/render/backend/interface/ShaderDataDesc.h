#pragma once

#include "ShaderData.h"
#include "ShaderFunctionDesc.h"

namespace gfx {
struct ShaderDataDesc {
    std::vector<ShaderFunctionDesc> functions;
    ShaderData data;
};
}
