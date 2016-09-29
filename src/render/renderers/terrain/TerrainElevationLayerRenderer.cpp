#include "TerrainElevationLayerRenderer.h"
#include "TerrainElevationTile.h"

void TerrainElevationLayerRenderer::OnInit() {}

void TerrainElevationLayerRenderer::Submit(const std::vector<TerrainDataTile*>& selected) {
    for (TerrainDataTile* baseTile : selected) {
        dg_assert_nm(baseTile->layerType == TerrainLayerType::Heightmap);
        TerrainElevationTile* tile = reinterpret_cast<TerrainElevationTile*>(baseTile);
    }
}
