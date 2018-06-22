#pragma once

#include "ConstantBuffer.h"
#include "GPUTileBuffer.h"
#include "MeshGeometry.h"
#include "TerrainDataTile.h"
#include "TileCache.h"
#include "DrawItem.h"

class GPUElevationDataTile : public TerrainDataTile {
public:
    GPUElevationDataTile(const TerrainTileKey& key, const glm::uvec2& res)
        : TerrainDataTile(key, TerrainLayerType::Heightmap, res) {}

    GPUTileSlot<gfx::PixelFormat::R32Float>* gpuData{nullptr};
    MeshGeometry*                            geometry{nullptr};
    ConstantBuffer*                          perTileConstants{nullptr};
    std::unique_ptr<const gfx::StateGroup>   stateGroup;
    gfx::DrawItemPtr                         drawItem;
};

class CPUElevationDataTile : public TerrainDataTile {
public:
    CPUElevationDataTile(const TerrainTileKey& key, const glm::uvec2& res)
        : TerrainDataTile(key, TerrainLayerType::Heightmap, res) {}
    CPUTileSlot<float>* cpuData{nullptr};
};
