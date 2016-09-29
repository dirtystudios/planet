#pragma once

#include <vector>
#include "TerrainDataTile.h"
#include "TerrainQuadNode.h"

class TerrainTileProducer {
private:
    TerrainLayerType _layerType;

public:
    TerrainTileProducer(TerrainLayerType layerType) : _layerType(layerType) {}

    TerrainLayerType LayerType() const { return _layerType; }

    virtual TerrainDataTile* GetTile(const TerrainQuadNode& quadNode) = 0;
    virtual bool GetTiles(const std::vector<const TerrainQuadNode*>& nodes, std::vector<TerrainDataTile*>* outputVector) = 0;
    virtual void Update() = 0;
};
