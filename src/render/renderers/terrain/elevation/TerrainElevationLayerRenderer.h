#pragma once

#include "ElevationDataTile.h"
#include "NormalDataTileProducer.h"
#include "RenderQueue.h"
#include "RenderView.h"
#include "StateGroup.h"
#include "TerrainLayerRenderer.h"
#include "TerrainQuadNode.h"

class TerrainElevationLayerRenderer : public TerrainLayerRenderer {
public:
    std::unique_ptr<const gfx::StateGroup> _base;
    DataTileSampler<GPUElevationDataTile>* _elevationSampler;
    DataTileSampler<GPUNormalsDataTile>*   _normalSampler;

public:
    TerrainElevationLayerRenderer(DataTileSampler<GPUElevationDataTile>* elevationSampler, DataTileSampler<GPUNormalsDataTile>* normalsSampler)
        : TerrainLayerRenderer(TerrainLayerType::Heightmap)
        , _elevationSampler(elevationSampler)
        , _normalSampler(normalsSampler) {}
    void         OnInit() final;
    virtual void Submit(RenderQueue* renderQueue, const FrameView* view, const std::vector<const TerrainQuadNode*>& selectedQuads) final;
};
