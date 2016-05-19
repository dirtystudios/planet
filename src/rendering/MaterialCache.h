#pragma once

#include <unordered_map>
#include <memory>
#include "Material.h"
#include "RenderDevice.h"

class MaterialCache {
private:
    graphics::RenderDevice* _device;
    std::string _baseDir;

    std::unordered_map<std::string, std::unique_ptr<Material>> _cache;
public:
    MaterialCache(graphics::RenderDevice* device, const std::string& baseDir);

    ~MaterialCache() {};

    Material* Get(const std::string& name, graphics::ShaderId ps);
};