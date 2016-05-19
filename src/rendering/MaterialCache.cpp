#include "MaterialCache.h"
#include "File.h"
#include "Image.h"
#include "MaterialImporter.h"

MaterialCache::MaterialCache(graphics::RenderDevice* device, const std::string& baseDir) : _device(device), _baseDir(baseDir) {
    if (!fs::IsPathDirectory(baseDir)) {
        LOG_E("%s", "Invalid Directory Path");
        _baseDir = fs::AppendPathProcessDir("/assets");
    }
}


Material* MaterialCache::Get(const std::string& name, graphics::ShaderId ps) {
    auto it = _cache.find(name);
    if (it != _cache.end()) {
        return it->second.get();
    }

    string fpath = _baseDir + "/" + name;
    MaterialData matData = materialImport::LoadMaterialDataFromFile(fpath);
    Image* img = matData.diffuseTexture.get();

    std::unique_ptr<Material> mat(new Material());

    graphics::TextureId texId = _device->CreateTexture2D(
        img->pixel_format == PixelFormat::RGB ? graphics::TextureFormat::RGB_U8 : graphics::TextureFormat::RGBA_U8,
        img->width,
        img->height,
        img->data);

    mat->diffuseTextures.push_back(texId);
    mat->Ka = _device->CreateShaderParam(ps, "Ka", graphics::ParamType::Float3);
    mat->Kd = _device->CreateShaderParam(ps, "Kd", graphics::ParamType::Float3);
    mat->Ks = _device->CreateShaderParam(ps, "Ks", graphics::ParamType::Float3);
    mat->Ke = _device->CreateShaderParam(ps, "Ke", graphics::ParamType::Float3);
    mat->Ns = _device->CreateShaderParam(ps, "Ns", graphics::ParamType::Float);

    mat->KaData = matData.Ka;
    mat->KdData = matData.Kd;
    mat->KeData = matData.Ke;
    mat->KsData = matData.Ks;
    mat->NsData = matData.Ns;

    _cache.insert(make_pair(name, std::move(mat)));
    return _cache[name].get();
}