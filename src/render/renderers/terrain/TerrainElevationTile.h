#pragma once

#include <glm/glm.hpp>
#include "CPUTileBuffer.h"
#include "GPUTileBuffer.h"
#include "TerrainDataTile.h"

class TerrainElevationTile : public TerrainDataTile {
public:
public:
    TerrainElevationTile(const TerrainTileKey& key) : TerrainDataTile(key, TerrainLayerType::Heightmap) {}

    CPUTileSlot<float>*                      cpuSlot{nullptr};
    GPUTileSlot<gfx::PixelFormat::R32Float>* gpuSlot{nullptr};
    glm::mat4                                transform;
};
