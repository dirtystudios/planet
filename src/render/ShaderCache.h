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
    gfx::RenderDevice* _device;
    std::string _baseDir;
    gfx::ShaderDataType _dataType;
    std::unordered_map<ShaderCacheKey, gfx::ShaderId> _shaderCache;

public:
    ShaderCache(gfx::RenderDevice* device, const std::string& baseDir) : _device(device), _baseDir(baseDir) {
        if (!fs::IsPathDirectory(baseDir)) {
            LOG_E("%s", "Invalid Directory Path given for ShaderDirectory");
            _baseDir = fs::AppendPathProcessDir("/shaders");
        }

        _baseDir += "/" + _device->DeviceConfig.DeviceAbbreviation;
        LOG_D("ShaderChache initialized (%s)", _baseDir.c_str());

        // TODO: determine if we will need set as Binary instead of Source
        _dataType = gfx::ShaderDataType::Source;
    }

    ~ShaderCache() {
        for (auto it : _shaderCache) {
            _device->DestroyResource(it.second);
        }
    }

    gfx::ShaderId Get(gfx::ShaderType shaderType, const std::string& name) {
        ShaderCacheKey key = BuildKey(shaderType, name);
        auto it = _shaderCache.find(key);
        if (it != _shaderCache.end()) {
            return it->second;
        }

        std::string fileContents = ReadFileContents(BuildFilePathForShader(shaderType, name));

        gfx::ShaderFunctionDesc functionDesc;
        functionDesc.functionName = name;
        functionDesc.entryPoint   = functionDesc.functionName;
        functionDesc.type         = shaderType;

        gfx::ShaderData shaderData;
        shaderData.type = _dataType;
        shaderData.data = fileContents.data();
        shaderData.len  = fileContents.size();

        gfx::ShaderId shader = _device->CreateShader(functionDesc, shaderData);
        assert(shader);

        _shaderCache.insert(std::make_pair(key, shader));
        return shader;
    }

private:
    std::string BuildFilePathForShader(gfx::ShaderType type, const std::string& name) {
        return _baseDir + "/" + name + BuildExtension(type);
    }
    std::string BuildExtension(gfx::ShaderType type) {
        std::string extension;
        switch (type) {
        case gfx::ShaderType::VertexShader:
            extension += "_vs";
            break;
        case gfx::ShaderType::PixelShader:
            extension += "_ps";
            break;
        default:
            assert(false);
        }
        extension += _device->DeviceConfig.ShaderExtension;
        ;
        return extension;
    }
    ShaderCacheKey BuildKey(gfx::ShaderType type, const std::string& name) {
        size_t h = 0;
        HashCombine(h, type);
        HashCombine(h, name);
        return h;
    }
};
