#include "MeshCache.h"
#include "File.h"
#include "MeshGeneration.h"
#include "MeshImporter.h"


MeshCache::MeshCache(gfx::RenderDevice* device, const std::string& baseDir) : _device(device), _baseDir(baseDir) {
    if (!fs::IsPathDirectory(baseDir)) {
        LOG_E("%s", "Invalid Directory Path");
        _baseDir = fs::AppendPathProcessDir("/assets");
    }
}

MeshCache::~MeshCache() {
    // TODO: Destroy meshes
}

MeshPtr MeshCache::Get(const std::string& name) {
    auto it = _cache.find(name);
    if (it != _cache.end()) {
        return it->second;
    }
    
    std::string fpath = _baseDir + "/" + name;

    std::vector<MeshPart> parts = meshImport::LoadMeshDataFromFile(fpath);
    auto inserted = _cache.emplace(name, std::make_shared<Mesh>(std::move(parts)));
    return inserted.first->second;
}
