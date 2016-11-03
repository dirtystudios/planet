#pragma once

#include <vector>
#include "TerrainDataTile.h"
#include "TerrainQuadNode.h"

template <typename T>
class DataTileSampler {
public:
    virtual T* FindTile(const TerrainTileKey& key) = 0;
};

class TerrainTileProducer {
private:
    TerrainLayerType _layerType;

public:
    TerrainTileProducer(TerrainLayerType layerType) : _layerType(layerType) {}

    TerrainLayerType LayerType() const { return _layerType; }

    virtual TerrainDataTile* GetTile(const TerrainQuadNode& quadNode)            = 0;
    virtual void Update(const std::vector<const TerrainQuadNode*>& nodesInScene) = 0;
};
