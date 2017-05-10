#pragma once

#include <vector>
#include "MetalEnumAdapter.h"
#include "ResourceManager.h"
#include "MetalResources.h"
#include "ResourceTypes.h"
#include "ShaderLibrary.h"
#include "ShaderType.h"

namespace gfx {

class MetalShaderLibrary : public ShaderLibrary, public Resource {
public:
    // TODO: ownership is messup right now. Open question of how to destroy
    // shaders. Do we even need to?
    std::vector<MetalLibraryFunction*> _functions;
    std::vector<id<MTLLibrary>>        _mtlLibraries;
    ResourceManager*                   _resourceManager{nullptr};

public:
    MetalShaderLibrary(ResourceManager* resourceManager) : _resourceManager(resourceManager) {}

    void AddLibrary(id<MTLLibrary> library);
    ShaderId GetShader(ShaderType type, const std::string& functionName);
};
}
