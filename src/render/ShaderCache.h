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
    gfx::ShaderDataType _dataType;
    std::unordered_map<ShaderCacheKey, gfx::ShaderId> _shaderCache;
    gfx::ShaderLibrary* _library;

public:
    ShaderCache(gfx::RenderDevice* device, const std::string& baseDir) : _device(device), _baseDir(baseDir) {
        if (!fs::IsPathDirectory(baseDir)) {
            LOG_E("%s", "Invalid Directory Path given for ShaderDirectory");
            _baseDir = fs::AppendPathProcessDir("/shaders");
        }

        _baseDir += "/" + _device->DeviceConfig.DeviceAbbreviation;
        LOG_D("ShaderChache initialized (%s)", _baseDir.c_str());

        std::vector<std::string> dirFiles = fs::ListFilesInDirectory(_baseDir);

        // TODO: determine if we will need set as Binary instead of Source
        _dataType = gfx::ShaderDataType::Source;
        std::vector<gfx::ShaderDataDesc> sdds;
        std::vector<std::string> allSrcs;

        for (const std::string& fname : dirFiles) {
            std::string absPath = _baseDir + "/" + fname;

            LOG_D("Creating shader for file:%s", fname.c_str());
            size_t nameBreak = fname.find_first_of("_");

            if (nameBreak == std::string::npos) {
                LOG_D("Incorrect name format for file:%s. Expected \"<function_name>_(vs|(ps|fs)).(glsl|hlsl|metal)\"",
                      fname.c_str());
                continue;
            }

            size_t fileBreak = fname.find_first_of(".");
            if (fileBreak == std::string::npos) {
                LOG_D("Incorrect name format for file:%s. Expected \"<function_name>_(vs|(ps|fs)).(glsl|hlsl|metal)\"",
                      fname.c_str());
                continue;
            }

            std::string shaderExt = fname.substr(nameBreak + 1, (fileBreak - nameBreak) - 1);
            gfx::ShaderType shaderType;
            if (!GetShaderTypeFromExt(shaderExt, &shaderType)) {
                LOG_D("Incorrect shader type for file:%s. Expected \"<function_name>_(vs|(ps|fs)).(glsl|hlsl|metal)\"",
                      fname.c_str());
                continue;
            }

            sdds.emplace_back();
            gfx::ShaderDataDesc& sdd = sdds.back();
            sdd.functions.emplace_back();

            gfx::ShaderFunctionDesc& fd = sdd.functions.back();
            fd.functionName             = fname.substr(0, nameBreak);
            fd.entryPoint               = fd.functionName;
            fd.type                     = shaderType;

            allSrcs.emplace_back();
            std::string& fileContentsBuffer = allSrcs.back();

            if (!fs::ReadFileContents(absPath, &fileContentsBuffer)) {
                LOG_D("Filed to read file contents for file:%s", fname.c_str());
                continue;
            }

            gfx::ShaderData& shaderData = sdd.data;
            shaderData.type             = _dataType;
            shaderData.data             = fileContentsBuffer.data();
            shaderData.len              = fileContentsBuffer.size();
        }
        _library = _device->CreateShaderLibrary(sdds);
    }

    ~ShaderCache() {
        for (auto it : _shaderCache) {
            _device->DestroyResource(it.second);
        }
    }

    gfx::ShaderId Get(gfx::ShaderType shaderType, const std::string& functionName) {
        gfx::ShaderId shader = _library->GetShader(shaderType, functionName);
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
