#pragma once

#include "RenderQueue.h"
#include "RenderView.h"
#include "StateGroup.h"
#include "ElevationDataTileProducer.h"
#include "TerrainLayerRenderer.h"
#include "TerrainQuadNode.h"

class TerrainElevationLayerRenderer : public TerrainLayerRenderer {
public:
    std::unique_ptr<const gfx::StateGroup> _base;
    DataTileSampler<ElevationDataTile>*    _elevationSampler;

public:
    TerrainElevationLayerRenderer(DataTileSampler<ElevationDataTile>* elevationSampler)
        : TerrainLayerRenderer(TerrainLayerType::Heightmap), _elevationSampler(elevationSampler) {}
    void         OnInit() final;
    virtual void Submit(RenderQueue* renderQueue, const FrameView* view, const std::vector<const TerrainQuadNode*>& selectedQuads) final;
};
