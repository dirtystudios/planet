#pragma once

#include "TerrainLayerType.h"
#include "TerrainTileKey.h"

class TerrainDataTile {
public:
    const TerrainLayerType layerType;
    const TerrainTileKey   key;

public:
    TerrainDataTile(uint32_t tid, uint32_t tx, uint32_t ty, uint32_t lod, TerrainLayerType layerType) : layerType(layerType), key({tid, tx, ty, lod}) {}
    TerrainDataTile(const TerrainTileKey& key, TerrainLayerType layerType) : layerType(layerType), key(key) {}
};
