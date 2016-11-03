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
    float    radius     = 256.f;
    uint32_t resolution = 128;

    //    std::shared_ptr<TerrainDeformation> deformation = std::make_shared<NoDeformation>();
    std::shared_ptr<TerrainDeformation> deformation = std::make_shared<SphericalDeformation>(radius);

    dm::Transform rootTransform;
    //    rootTransform.rotateDegrees(-25, glm::normalize(glm::vec3(1, 1, 1)));
    //    rootTransform.translate(glm::vec3(0, 10, 0));
    glm::mat4 rootMatrix = rootTransform.matrix();

    dm::Transform topTransform;
    topTransform.rotateDegrees(-90, glm::vec3(1, 0, 0));
    topTransform.translate(glm::vec3(0, radius / 2.f, 0));
    _topTree = std::make_shared<TerrainQuadTree>(radius, deformation, rootMatrix * topTransform.matrix());
    _selectors.emplace_back(new TerrainQuadNodeSelector(_topTree));
    //
    dm::Transform bottomTransform;
    bottomTransform.rotateDegrees(90, glm::vec3(1, 0, 0));
    bottomTransform.translate(glm::vec3(0, -radius / 2.f, 0));
    _bottomTree = std::make_shared<TerrainQuadTree>(radius, deformation, rootMatrix * bottomTransform.matrix());
    _selectors.emplace_back(new TerrainQuadNodeSelector(_bottomTree));

    dm::Transform frontTransform;
    frontTransform.translate(glm::vec3(0, 0, radius / 2.f));
    _frontTree = std::make_shared<TerrainQuadTree>(radius, deformation, rootMatrix * frontTransform.matrix());
    _selectors.emplace_back(new TerrainQuadNodeSelector(_frontTree));

    dm::Transform backTransform;
    backTransform.rotateDegrees(180, glm::vec3(1, 0, 0));
    backTransform.translate(glm::vec3(0, 0, -radius / 2.f));
    _backTree = std::make_shared<TerrainQuadTree>(radius, deformation, rootMatrix * backTransform.matrix());
    _selectors.emplace_back(new TerrainQuadNodeSelector(_backTree));

    dm::Transform leftTransform;
    leftTransform.rotateDegrees(-90, glm::vec3(0, 1, 0));
    leftTransform.translate(glm::vec3(-radius / 2.f, 0, 0));
    _leftTree = std::make_shared<TerrainQuadTree>(radius, deformation, rootMatrix * leftTransform.matrix());
    _selectors.emplace_back(new TerrainQuadNodeSelector(_leftTree));

    dm::Transform rightTransform;
    rightTransform.rotateDegrees(90, glm::vec3(0, 1, 0));
    rightTransform.translate(glm::vec3(radius / 2.f, 0, 0));
    _rightTree = std::make_shared<TerrainQuadTree>(radius, deformation, rootMatrix * rightTransform.matrix());
    _selectors.emplace_back(new TerrainQuadNodeSelector(_rightTree));

    _layers.elevation.producer.reset(new TerrainElevationTileProducer(device(), {resolution, resolution}));
    _layers.elevation.renderer.reset(new TerrainElevationLayerRenderer(_layers.elevation.producer.get()));

    _tileProducers.push_back(_layers.elevation.producer.get());
    _layerRenderers.push_back(_layers.elevation.renderer.get());

    for (TerrainLayerRenderer* layerRenderer : _layerRenderers) {
        layerRenderer->Init(device(), services());
    }
}

void TerrainRenderer::Submit(RenderQueue* renderQueue, RenderView* renderView) {
    _nodesInScene.clear();

    //    _bottomTree->transform() = glm::rotate(_bottomTree->transform(), 0.01f, glm::vec3(0, 0, 1));
    //    _frontTree->transform() = glm::rotate(_frontTree->transform(), 0.01f, glm::vec3(0, 0, 1));

    for (std::unique_ptr<TerrainQuadNodeSelector>& selector : _selectors) {
        selector->SelectQuadNodes(*renderView->camera, &_nodesInScene);
    }

    //    for (const TerrainQuadNode* quad : _nodesInScene) {
    //        dm::Rect3Dd worldRect = quad->worldRect();
    //        services()->debugDraw()->AddRect3D(worldRect, dutil::getColor(quad->key.lod), false);
    //    }

    for (TerrainTileProducer* producer : _tileProducers) {
        producer->Update(_nodesInScene);
    }

    for (TerrainLayerRenderer* layer : _layerRenderers) {
        layer->Submit(renderQueue, renderView, _nodesInScene);
    }
}
