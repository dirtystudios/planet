#pragma once

#include "ConstantBuffer.h"
#include "GPUTileBuffer.h"
#include "MeshGeometry.h"
#include "TerrainDataTile.h"
#include "TileCache.h"

class ElevationDataTile : public TerrainDataTile {
public:
    ElevationDataTile(const TerrainTileKey& key) : TerrainDataTile(key, TerrainLayerType::Heightmap) {}

    CPUTileSlot<float>*                      cpuData{nullptr};
    GPUTileSlot<gfx::PixelFormat::R32Float>* gpuData{nullptr};
    MeshGeometry*                            geometry{nullptr};
    ConstantBuffer*                          perTileConstants{nullptr};
    std::unique_ptr<const gfx::StateGroup>   stateGroup;
    gfx::DrawItemPtr                         drawItem;
};
