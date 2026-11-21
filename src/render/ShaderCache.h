#pragma once

#include <unordered_map>
#include <cassert>
#include <iostream>
#include "File.h"
#include "WatchDirManager.h"
#include "ShaderType.h"
#include "Helpers.h"
#include "DGAssert.h"

class ShaderCache {
private:
    using ShaderCacheKey = size_t;
    gfx::RenderDevice* _device;
    fs::path _baseDir;
    fs::WatcherId _watcherId;

public:
    ShaderCache(gfx::RenderDevice* device, const std::string& baseDir) : _device(device), _baseDir(baseDir) {
        if (_baseDir.empty()) {
            _baseDir = fs::GetProcessDirectory() / "assets";
        }
        else if (_baseDir.is_relative()) {
            _baseDir = fs::GetProcessDirectory() / _baseDir;
        }

        _baseDir += _device->DeviceConfig.ShaderDir;
        LOG_D("ShaderChache initialized ({})", _baseDir);

        fs::FileEventDelegate eventCallback = [&](const fs::FileEvent& event) {
            std::string fileContents;
            if (!fs::ReadFileContents(event.fpath, &fileContents)) {
                LOG_D("Filed to read file contents for file:{}", event.fpath);
                return;
            }
            
            gfx::ShaderData shaderData;
            shaderData.type             = gfx::ShaderDataType::Source;
            shaderData.data             = fileContents.data();
            shaderData.len              = fileContents.size();
            shaderData.name             = event.fpath.string();
            _device->AddOrUpdateShaders({shaderData});
        };
        
        if(_device->GetDeviceApi() != gfx::RenderDeviceApi::OpenGL) {
            _watcherId = fs::WatchDirManager::AddWatcher(_baseDir, eventCallback);
        }

        // TODO: determine if we will need set as Binary instead of Source
        gfx::ShaderDataType dataType = gfx::ShaderDataType::Source;
        std::vector<gfx::ShaderData> datas;
        std::vector<std::string> allSrcs;

        for (const auto& entry : fs::directory_iterator(_baseDir)) {
            if (!entry.is_regular_file())
                continue;
            auto fname = entry.path().filename().string();
            LOG_D("Creating shader for file:{}", fname);

            allSrcs.emplace_back();
            std::string& fileContentsBuffer = allSrcs.back();

            if (!fs::ReadFileContents(entry, &fileContentsBuffer)) {
                LOG_D("Filed to read file contents for file:{}", fname);
                continue;
            }

            datas.emplace_back();
            gfx::ShaderData& shaderData = datas.back();
            shaderData.type             = dataType;
            shaderData.data             = fileContentsBuffer.data();
            shaderData.len              = fileContentsBuffer.size();
            shaderData.name             = fname;
        }
        _device->AddOrUpdateShaders(datas);
    }

    ~ShaderCache() { fs::WatchDirManager::RemoveWatcher(_watcherId); }

    gfx::ShaderId Get(gfx::ShaderType shaderType, const std::string& functionName) {
        gfx::ShaderId shader = _device->GetShader(shaderType, functionName);
        dg_assert((shader > 0), "Failed to get shader (type:%s, functionName:%s)", ToString(shaderType).c_str(),
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
