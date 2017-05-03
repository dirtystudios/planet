#pragma once

#include "TerrainLayerType.h"
#include "TerrainTileKey.h"

class TerrainDataTile {
public:
    const TerrainLayerType layerType;
    const TerrainTileKey   key;
    const glm::uvec2       resolution;

public:
    TerrainDataTile(uint32_t tid, uint32_t tx, uint32_t ty, uint32_t lod, TerrainLayerType layerType, const glm::uvec2& res)
        : TerrainDataTile({tid, tx, ty, lod}, layerType, res) {}

    TerrainDataTile(const TerrainTileKey& key, TerrainLayerType layerType, const glm::uvec2& res)
        : layerType(layerType)
        , key(key)
        , resolution(res) {}
};
