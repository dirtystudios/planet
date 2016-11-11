#include "MaterialCache.h"
#include "File.h"
#include "Image.h"
#include "MaterialImporter.h"

MaterialCache::MaterialCache(gfx::RenderDevice* device, const std::string& baseDir) : _device(device), _baseDir(baseDir) {
    if (!fs::IsPathDirectory(baseDir)) {
        LOG_E("%s", "Invalid Directory Path");
        _baseDir = fs::AppendPathProcessDir("/assets");
    }
}

Material* MaterialCache::Get(const std::string& name, gfx::ShaderId ps) {
    auto it = _cache.find(name);
    if (it != _cache.end()) {
        return &it->second;
    }

    std::string  fpath   = _baseDir + "/" + name;
    std::vector<MaterialData> matData = materialImport::LoadMaterialDataFromFile(fpath);

	Material mat;
	mat.matData = std::move(matData);
	mat.pixelShader = ps;
	auto inserted = _cache.emplace(name, std::move(mat));

	return &inserted.first->second;
}
