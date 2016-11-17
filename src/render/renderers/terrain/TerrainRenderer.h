#pragma once

#include <memory>
#include <set>
#include <vector>
#include "DataTileProducer.h"
#include "Renderer.h"
#include "TerrainLayerRenderer.h"
#include "TerrainQuadNodeSelector.h"
#include "TerrainQuadTree.h"

class TerrainElevationLayerRenderer;
class ElevationDataTileProducer;
class NormalDataTileProducer;

template <typename Producer, typename Renderer>
struct Layer {
    std::unique_ptr<Producer> producer;
    std::unique_ptr<Renderer> renderer;
};

struct Layers {
    Layer<ElevationDataTileProducer, TerrainElevationLayerRenderer> elevation;
    Layer<NormalDataTileProducer, TerrainElevationLayerRenderer>    normals;
};

class TerrainRenderer : public Renderer {
private:
    Layers _layers;

    std::vector<TerrainQuadTree*> _terrainCache;

    std::shared_ptr<TerrainQuadTree> _bottomTree;
    std::shared_ptr<TerrainQuadTree> _topTree;
    std::shared_ptr<TerrainQuadTree> _frontTree;
    std::shared_ptr<TerrainQuadTree> _backTree;
    std::shared_ptr<TerrainQuadTree> _leftTree;
    std::shared_ptr<TerrainQuadTree> _rightTree;

    std::vector<TerrainQuadTreePtr>                       _terrains;
    std::vector<std::unique_ptr<TerrainQuadNodeSelector>> _selectors;
    std::vector<DataTileProducer*>                        _tileProducers;
    std::vector<TerrainLayerRenderer*>                    _layerRenderers;

    std::vector<const TerrainQuadNode*> _nodesInScene;

    std::set<TerrainTileKey> _keysInPrevScene;
    std::set<TerrainTileKey> _keysInScene;
    std::set<TerrainTileKey> _keysLeaving;
    std::set<TerrainTileKey> _keysEntering;

public:
    TerrainRenderer();
    ~TerrainRenderer();

    void OnInit() override;
    void Submit(RenderQueue* renderQueue, const FrameView* view) final;
};
