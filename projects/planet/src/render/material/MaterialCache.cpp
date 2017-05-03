#include "MaterialCache.h"
#include "File.h"
#include "Config.h"
#include "MaterialImporter.h"

MaterialCache::MaterialCache(gfx::RenderDevice* device, const std::string& baseDir) : _device(device), _baseDir(baseDir) {
    if (!fs::IsPathDirectory(baseDir)) {
        LOG_E("%s", "Invalid Directory Path");
        _baseDir = fs::AppendPathProcessDir("/assets");
    }
}

Material* MaterialCache::Get(const std::string& name) {
    auto it = _cache.find(name);
    if (it != _cache.end()) {
        return &it->second;
    }

    std::string assetDirPath = config::Config::getInstance().GetConfigString("RenderDeviceSettings", "AssetDirectory");
    if (!fs::IsPathDirectory(assetDirPath)) {
        LOG_E("Invalid Directory Path given for AssetDirectory.");
    }

    std::string fpath   = _baseDir + "/" + name;
    std::vector<MaterialData> matData = materialImport::LoadMaterialDataFromFile(fpath);

    Material mat;
    mat.matData = std::move(matData);
    auto inserted = _cache.emplace(name, std::move(mat));

    return &inserted.first->second;
}