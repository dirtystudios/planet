#pragma once

#include <memory>
#include "Renderer.h"
#include "TerrainLayerRenderer.h"
#include "TerrainQuadNodeSelector.h"
#include "TerrainQuadTree.h"
#include "TerrainTileProducer.h"

class TerrainElevationLayerRenderer;
class TerrainElevationTileProducer;

template <typename Producer, typename Renderer>
struct Layer {
    std::unique_ptr<Producer> producer;
    std::unique_ptr<Renderer> renderer;
};

struct Layers {
    Layer<TerrainElevationTileProducer, TerrainElevationLayerRenderer> elevation;
};

class TerrainRenderer : public Renderer {
private:
    Layers _layers;

    std::shared_ptr<TerrainQuadTree> _tree;

    std::unique_ptr<TerrainQuadNodeSelector> _selector;
    std::vector<TerrainTileProducer*>        _tileProducers;
    std::vector<TerrainLayerRenderer*>       _layerRenderers;

    std::vector<const TerrainQuadNode*> _selectedQuadNodes;
    std::array<std::vector<TerrainDataTile*>, kTerrainLayerTypeCount> _selectedTilesByLayer;

public:
    TerrainRenderer();
    ~TerrainRenderer();

    void OnInit() override;
    void Submit(RenderQueue* renderQueue, RenderView* renderView) final;
};
