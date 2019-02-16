#pragma once

#include <memory>
#include <set>
#include <vector>
#include "DataTileProducer.h"
#include "Renderer.h"
#include "TerrainLayerRenderer.h"
#include "TerrainQuadNodeSelector.h"
#include "TerrainQuadTree.h"
#include "TerrainRenderObj.h"

class TerrainElevationLayerRenderer;

class CPUElevationDataTileProducer;
class CPUNormalDataTileProducer;
class NormalDataTileProducer;
class ElevationDataTileProducer;

class TerrainRenderer : public TypedRenderer<TerrainRenderObj> {
private:
    struct {
        struct {
            std::unique_ptr<CPUElevationDataTileProducer> cpu;
            std::unique_ptr<ElevationDataTileProducer> gpu;
        } elevations;
        struct {
            std::unique_ptr<CPUNormalDataTileProducer> cpu;
            std::unique_ptr<NormalDataTileProducer> gpu;
        } normals;
    } _producers;

    struct {
        std::unique_ptr<TerrainElevationLayerRenderer> baseLayer;
    } _renderers;

    std::vector<TerrainRenderObj*> _renderObjs;
    std::vector<TerrainQuadTree*> _terrains;

    std::vector<DataTileProducer*> _tileProducers;
    std::vector<TerrainLayerRenderer*> _layerRenderers;

    std::vector<const TerrainQuadNode*> _nodesInScene;

    std::set<TerrainTileKey> _keysInPrevScene;
    std::set<TerrainTileKey> _keysInScene;
    std::set<TerrainTileKey> _keysLeaving;
    std::set<TerrainTileKey> _keysEntering;

public:
    TerrainRenderer();
    ~TerrainRenderer();

    void OnInit() override;
    void Register(TerrainRenderObj* renderObj) final;
    void Unregister(TerrainRenderObj* renderObj) final { assert(false); }
    void Submit(RenderQueue* renderQueue, const FrameView* view) final;
    void Submit(ComputeQueue* renderQueue, const FrameView* view) final {};
};
