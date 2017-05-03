#pragma once

#include <unordered_map>
#include <memory>
#include "Material.h"
#include "RenderDevice.h"

class MaterialCache {
private:
    gfx::RenderDevice* _device;
    std::string _baseDir;

    std::unordered_map<std::string, Material> _cache;
public:
    MaterialCache(gfx::RenderDevice* device, const std::string& baseDir);

    ~MaterialCache() {};

    Material* Get(const std::string& name);
};
