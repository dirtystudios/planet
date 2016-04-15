#pragma once

#include <unordered_map>
#include <cassert>
#include <iostream>
#include "Mesh.h"
#include "RenderDevice.h"

class MeshCache {
private:    
    graphics::RenderDevice* _device;
    std::string _baseDir;

    std::unordered_map<std::string, Mesh*> _cache;    
public:
    MeshCache(graphics::RenderDevice* device, const std::string& baseDir);

    ~MeshCache();

    Mesh* Get(const std::string& name);

private:    
};