#pragma once

#include <unordered_map>
#include <memory>
#include "Material.h"
#include "RenderDevice.h"

class MaterialCache {
private:
    gfx::RenderDevice* _device;
    std::string _baseDir;

    std::unordered_map<std::string, MaterialPtr> _cache;
public:
    MaterialCache(gfx::RenderDevice* device, const std::string& baseDir);

    ~MaterialCache() {};

    MaterialPtr Get(const std::string& name);
    MaterialPtr Create(const std::string& name, const std::string& assetPath);
    MaterialPtr Create(const std::string& name, MaterialData&& materialData);
};
