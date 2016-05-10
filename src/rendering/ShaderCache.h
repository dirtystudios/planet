#pragma once

#include <unordered_map>
#include <cassert>
#include <iostream>
#include "File.h"
#include "ShaderType.h"
#include "Helpers.h"

class ShaderCache {
private:
    using ShaderCacheKey = size_t;
    graphics::RenderDevice* _device;
    std::string _baseDir;

    std::unordered_map<ShaderCacheKey, graphics::ShaderId> _shaderCache;

public:
    ShaderCache(graphics::RenderDevice* device, const std::string& baseDir) : _device(device), _baseDir(baseDir) {
        if (!fs::IsPathDirectory(baseDir)) {
            LOG_E("%s", "Invalid Directory Path given for ShaderDirectory");
        }

        _baseDir += "/" + _device->DeviceConfig.DeviceAbbreviation;
        LOG_D("ShaderChache initialized (%s)", _baseDir.c_str());
    }

    ~ShaderCache() {
        for (auto it : _shaderCache) {
            _device->DestroyShader(it.second);
        }
    }

    graphics::ShaderId Get(graphics::ShaderType shaderType, const std::string& name) {
        ShaderCacheKey key = BuildKey(shaderType, name);
        auto it = _shaderCache.find(key);
        if (it != _shaderCache.end()) {
            return it->second;
        }

        std::string fileContents  = ReadFileContents(BuildFilePathForShader(shaderType, name));
        graphics::ShaderId shader = _device->CreateShader(shaderType, fileContents);
        assert(shader);

        _shaderCache.insert(make_pair(key, shader));
        return shader;
    }

private:
    std::string BuildFilePathForShader(graphics::ShaderType type, const std::string& name) {
        return _baseDir + "/" + name + BuildExtension(type);
    }
    std::string BuildExtension(graphics::ShaderType type) {
        std::string extension;
        switch (type) {
        case graphics::ShaderType::VertexShader:
            extension += "_vs";
            break;
        case graphics::ShaderType::PixelShader:
            extension += "_ps";
            break;
        default:
            assert(false);
        }
        extension += _device->DeviceConfig.ShaderExtension;
        ;
        return extension;
    }
    ShaderCacheKey BuildKey(graphics::ShaderType type, const std::string& name) {
        size_t h = 0;
        HashCombine(h, type);
        HashCombine(h, name);
        return h;
    }
};