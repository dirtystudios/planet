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
        return it->second.get();
    }

    std::string  fpath   = _baseDir + "/" + name;
    MaterialData matData = materialImport::LoadMaterialDataFromFile(fpath);
    Image*       img     = matData.diffuseTexture.get();

    std::unique_ptr<Material> mat(new Material());

    gfx::TextureId texId =
        _device->CreateTexture2D(img->pixel_format == PixelFormat::RGB ? gfx::PixelFormat::RGB8Unorm : gfx::PixelFormat::RGBA8Unorm, img->width, img->height, img->data);

    mat->diffuseTextures.push_back(texId);
    mat->KaData = matData.Ka;
    mat->KdData = matData.Kd;
    mat->KeData = matData.Ke;
    mat->KsData = matData.Ks;
    mat->NsData = matData.Ns;

    _cache.insert(make_pair(name, std::move(mat)));
    return _cache[name].get();
}
