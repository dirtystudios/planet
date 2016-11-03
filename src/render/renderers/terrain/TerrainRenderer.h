#pragma once

#include <memory>
#include <vector>
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

class TerrainRenderObj {
public:
};

class TerrainRenderer : public Renderer {
private:
    Layers _layers;

    std::shared_ptr<TerrainQuadTree> _bottomTree;
    std::shared_ptr<TerrainQuadTree> _topTree;
    std::shared_ptr<TerrainQuadTree> _frontTree;
    std::shared_ptr<TerrainQuadTree> _backTree;
    std::shared_ptr<TerrainQuadTree> _leftTree;
    std::shared_ptr<TerrainQuadTree> _rightTree;

    std::vector<TerrainQuadTreePtr>                       _terrains;
    std::vector<std::unique_ptr<TerrainQuadNodeSelector>> _selectors;
    std::vector<TerrainTileProducer*>                     _tileProducers;
    std::vector<TerrainLayerRenderer*>                    _layerRenderers;

    std::vector<const TerrainQuadNode*> _nodesInScene;

public:
    TerrainRenderer();
    ~TerrainRenderer();

    void OnInit() override;
    void Submit(RenderQueue* renderQueue, RenderView* renderView) final;
};
