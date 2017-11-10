#pragma once

#include "Material.h"
#include "RenderCache.h"
#include "MaterialImporter.h"

struct MaterialCachePolicy {
    using CacheItemType = MaterialPtr;
    using FileDataType = std::vector<MaterialData>;

    MaterialCachePolicy(gfx::RenderDevice* device) {};

    FileDataType LoadDataFromFile(const std::string& fpath) {
        return materialImport::LoadMaterialDataFromFile(fpath);
    }

    CacheItemType ConstructCacheItem(FileDataType&& data) {
        return std::make_shared<Material>(std::move(data));
    }
};

using MaterialCache = RenderCache<MaterialCachePolicy>;