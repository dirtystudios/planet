#pragma once

#include <unordered_map>
#include <cassert>
#include "Mesh.h"
#include "RenderDevice.h"

class MeshCache {
private:    
    gfx::RenderDevice* _device;
    std::string _baseDir;

    std::unordered_map<std::string, MeshPtr> _cache;
public:
    MeshCache(gfx::RenderDevice* device, const std::string& baseDir);

    ~MeshCache();

    MeshPtr Get(const std::string& name);
    MeshPtr Create(const std::string& name, const std::string& assetPath);
    MeshPtr Create(const std::string& name, MeshGeometryData&& geometryData);

private:    
};
