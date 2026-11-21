#pragma once

#include <unordered_map>
#include "RenderDevice.h"
#include "File.h"

template<class Policy>
class RenderCache {
    using CacheItem = typename Policy::CacheItemType;
    using FileData = typename Policy::FileDataType;
private:
    gfx::RenderDevice* _device;
    fs::path _baseDir;

    std::unordered_map<std::string, CacheItem> _cache;
    Policy _policy;
public:
    RenderCache(gfx::RenderDevice* device, const std::string& baseDir)
        : _device(device), _baseDir(baseDir), _policy(device) {
        if (_baseDir.empty()) {
            _baseDir = fs::GetProcessDirectory() / "assets";
        }
        else if (_baseDir.is_relative()) {
            _baseDir = fs::GetProcessDirectory() / _baseDir;
        }
    }

    ~RenderCache() { /* todo: destroy meshes */ }

    CacheItem Get(const std::string& name) {
        auto it = _cache.find(name);
        if (it != _cache.end())
            return it->second;
        return nullptr;
    }

    CacheItem Create(const std::string& name, const fs::path& assetPath) {
        CacheItem mesh = Get(name);
        if (mesh)
            return mesh;

        auto fpath = _baseDir / assetPath;

        FileData data = _policy.LoadDataFromFile(fpath);
        auto ins = _cache.emplace(name, _policy.ConstructCacheItem(std::move(data)));
        return ins.first->second;
    }
};