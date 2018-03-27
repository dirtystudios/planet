#pragma once

#include <set>
#include <vector>
#include "TerrainDataTile.h"
#include "TerrainQuadNode.h"

template <typename T>
class DataTileSampler {
public:
    // return the closest matching tile for the given key
    virtual T* FindTile(const TerrainTileKey& key) = 0;
};

class DataTileProducer {
public:
    const TerrainLayerType layerType;
    const glm::uvec2       tileResolution;

public:
    DataTileProducer(TerrainLayerType layerType, const glm::uvec2& resolution)
        : layerType(layerType)
        , tileResolution(resolution) {}

    virtual TerrainDataTile* GetTile(const TerrainQuadNode& quadNode) = 0;
    virtual void Update(const std::vector<const TerrainQuadNode*>& nodesInScene, const std::set<TerrainTileKey>& keysLeaving,
                        const std::set<TerrainTileKey>& keysEntering) = 0;
};
