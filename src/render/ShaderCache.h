#pragma once

#include <unordered_map>
#include <cassert>
#include <iostream>
#include "File.h"
#include "ShaderType.h"
#include "Helpers.h"
#include "DGAssert.h"

class ShaderCache {
private:
    using ShaderCacheKey = size_t;
    gfx::RenderDevice* _device;
    std::string _baseDir;
    fs::WatcherId _watcherId;

public:
    ShaderCache(gfx::RenderDevice* device, const std::string& baseDir) : _device(device), _baseDir(baseDir) {
        if (!fs::IsPathDirectory(baseDir)) {
            LOG_E("%s", "Invalid Directory Path given for ShaderDirectory");
            _baseDir = fs::AppendPathProcessDir("/shaders");
        }

        _baseDir += "/" + _device->DeviceConfig.DeviceAbbreviation;
        LOG_D("ShaderChache initialized (%s)", _baseDir.c_str());

        auto eventCallback = [&](const fs::FileEvent& event) {
            std::string fileContents;
            if (!fs::ReadFileContents(event.fpath, &fileContents)) {
                LOG_D("Filed to read file contents for file:%s", event.fpath.c_str());
                return;
            }
            
            gfx::ShaderData shaderData;
            shaderData.type             = gfx::ShaderDataType::Source;
            shaderData.data             = fileContents.data();
            shaderData.len              = fileContents.size();
            _device->AddOrUpdateShaders({shaderData});
        };
        _watcherId = fs::WatchDir(_baseDir, eventCallback);

        std::vector<std::string> dirFiles = fs::ListFilesInDirectory(_baseDir);

        // TODO: determine if we will need set as Binary instead of Source
        gfx::ShaderDataType dataType = gfx::ShaderDataType::Source;
        std::vector<gfx::ShaderData> datas;
        std::vector<std::string> allSrcs;

        for (const std::string& fname : dirFiles) {
            std::string absPath = _baseDir + "/" + fname;

            LOG_D("Creating shader for file:%s", fname.c_str());

            allSrcs.emplace_back();
            std::string& fileContentsBuffer = allSrcs.back();

            if (!fs::ReadFileContents(absPath, &fileContentsBuffer)) {
                LOG_D("Filed to read file contents for file:%s", fname.c_str());
                continue;
            }

            datas.emplace_back();
            gfx::ShaderData& shaderData = datas.back();
            shaderData.type             = dataType;
            shaderData.data             = fileContentsBuffer.data();
            shaderData.len              = fileContentsBuffer.size();
        }
        _device->AddOrUpdateShaders(datas);
    }

    ~ShaderCache() { fs::StopWatching(_watcherId); }

    gfx::ShaderId Get(gfx::ShaderType shaderType, const std::string& functionName) {
        gfx::ShaderId shader = _device->GetShader(shaderType, functionName);
        dg_assert(shader, "Failed to get shader (type:%s, functionName:%s)", ToString(shaderType).c_str(),
                  functionName.c_str());
        return shader;
    }

private:
    bool GetShaderTypeFromExt(const std::string& ext, gfx::ShaderType* typeOut) {
        if (ext == "vs") {
            *typeOut = gfx::ShaderType::VertexShader;
        } else if (ext == "ps" || ext == "fs") {
            *typeOut = gfx::ShaderType::PixelShader;
        } else {
            return false;
        }
        return true;
    }
};
