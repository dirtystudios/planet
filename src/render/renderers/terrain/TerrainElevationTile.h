#pragma once

#include <glm/glm.hpp>
#include "CPUTileBuffer.h"
#include "ConstantBuffer.h"
#include "GPUTileBuffer.h"
#include "MeshGeometry.h"
#include "StateGroup.h"
#include "TerrainDataTile.h"

class TerrainElevationTile : public TerrainDataTile {
public:
public:
    TerrainElevationTile(const TerrainTileKey& key) : TerrainDataTile(key, TerrainLayerType::Heightmap) {}

    CPUTileSlot<float>*                      cpuSlot{nullptr};
    GPUTileSlot<gfx::PixelFormat::R32Float>* gpuSlot{nullptr};
    glm::mat4                                transform; // this should not be on the datatile
    glm::uvec2                               resolution;
    MeshGeometry*                            geometry{nullptr};
    ConstantBuffer*                          perTileConstants{nullptr};
    std::unique_ptr<const gfx::StateGroup>   stateGroup;
    gfx::DrawItemPtr                         drawItem;
};
