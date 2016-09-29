#include "TerrainRenderer.h"

#include <glm/gtx/transform.hpp>
#include <queue>
#include "DMath.h"
#include "GPUTextureArrayBuffer.h"
#include "Log.h"
#include "Log.h"
#include "MeshGeneration.h"
#include "Spatial.h"
#include "TerrainDataTile.h"
#include "TerrainElevationLayerRenderer.h"
#include "TerrainElevationTileProducer.h"
#include "TerrainQuadNode.h"

TerrainRenderer::TerrainRenderer() {}

TerrainRenderer::~TerrainRenderer() {}

void TerrainRenderer::OnInit() {
    dm::Transform transform;
    transform.rotateDegrees(90, glm::vec3(1, 0, 0));
    transform.translate(glm::vec3(0, -256, 0));

    _tree = std::make_shared<TerrainQuadTree>(256, transform.matrix());
    _selector.reset(new TerrainQuadNodeSelector(_tree));

    _layers.elevation.renderer.reset(new TerrainElevationLayerRenderer());
    _layers.elevation.producer.reset(new TerrainElevationTileProducer(device(), {256, 256}));

    _layerRenderers.push_back(_layers.elevation.renderer.get());
    _tileProducers.push_back(_layers.elevation.producer.get());

    for (TerrainLayerRenderer* layerRenderer : _layerRenderers) {
        layerRenderer->Init(services());
    }
}

void TerrainRenderer::Submit(RenderQueue* renderQueue, RenderView* renderView) {
    _selectedQuadNodes.clear();
    for (std::vector<TerrainDataTile*>& v : _selectedTilesByLayer) {
        v.clear();
    }

    _selector->SelectQuadNodes(*renderView->camera, &_selectedQuadNodes);
    for (const TerrainQuadNode* quad : _selectedQuadNodes) {
        services()->debugDraw()->AddRect3D(quad->worldRect(), dutil::getColor(quad->key.lod), false);
    }

    for (TerrainTileProducer* producer : _tileProducers) {
        producer->Update();
    }

    for (TerrainTileProducer* producer : _tileProducers) {
        producer->GetTiles(_selectedQuadNodes, &_selectedTilesByLayer[static_cast<uint32_t>(producer->LayerType())]);
    }

    for (TerrainLayerRenderer* layer : _layerRenderers) {
        layer->Submit(_selectedTilesByLayer[static_cast<uint32_t>(layer->LayerType())]);
    }
}
