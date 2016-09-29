#pragma once

#include "TerrainLayerRenderer.h"

class TerrainElevationLayerRenderer : public TerrainLayerRenderer {
public:
    TerrainElevationLayerRenderer() : TerrainLayerRenderer(TerrainLayerType::Heightmap) {}
    void OnInit() final;
    void Submit(const std::vector<TerrainDataTile*>& selected) final;
};
