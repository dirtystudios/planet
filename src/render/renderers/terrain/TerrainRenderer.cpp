#include "TerrainRenderer.h"

#include <glm/gtx/transform.hpp>
#include <queue>
#include <set>
#include "DMath.h"
#include "ElevationDataTileProducer.h"
#include "Log.h"
#include "Log.h"
#include "MeshGeneration.h"
#include "NormalDataTileProducer.h"
#include "Spatial.h"
#include "TerrainDataTile.h"
#include "TerrainElevationLayerRenderer.h"
#include "TerrainQuadNode.h"

TerrainRenderer::TerrainRenderer() {}

TerrainRenderer::~TerrainRenderer() {}

std::unique_ptr<CPUElevationDataTileProducer> residualsProducer;
std::unique_ptr<CPUNormalDataTileProducer>    normalsProducer;

// TODO: Layer abstraction in TerrainRenderer is no good. Think of something better.
// TODO: Producers have alot of duplicate code. Do something about it
// TODO: Properly rendering partial tiles. (ex. Scale texure coords). Necessary to fix flickering
// TODO: producers need to walk tree instead of jumping straight to tile so that we have high lod fallback
// TODO: selection can select too many tiles, raping caches

void TerrainRenderer::OnInit() {
    float    radius     = 10000;
    uint32_t resolution = 128;

    std::shared_ptr<TerrainDeformation> deformation = std::make_shared<NoDeformation>();
    // std::shared_ptr<TerrainDeformation> deformation = std::make_shared<SphericalDeformation>(radius);

    dm::Transform rootTransform;
    //    rootTransform.rotateDegrees(-25, glm::normalize(glm::vec3(1, 1, 1)));
    //    rootTransform.translate(glm::vec3(0, 10, 0));
    glm::mat4 rootMatrix = rootTransform.matrix();

    dm::Transform topTransform;
    //    topTransform.rotateDegrees(-90, glm::vec3(1, 0, 0));
    //    topTransform.translate(glm::vec3(0, radius / 2.f, 0));
    _topTree = std::make_shared<TerrainQuadTree>(radius, deformation, rootMatrix * topTransform.matrix());
    _selectors.emplace_back(new TerrainQuadNodeSelector(_topTree));

    //    dm::Transform bottomTransform;
    //    bottomTransform.rotateDegrees(90, glm::vec3(1, 0, 0));
    //    bottomTransform.translate(glm::vec3(0, -radius / 2.f, 0));
    //    _bottomTree = std::make_shared<TerrainQuadTree>(radius, deformation, rootMatrix * bottomTransform.matrix());
    //    _selectors.emplace_back(new TerrainQuadNodeSelector(_bottomTree));
    //
    //    dm::Transform frontTransform;
    //    frontTransform.translate(glm::vec3(0, 0, radius / 2.f));
    //    _frontTree = std::make_shared<TerrainQuadTree>(radius, deformation, rootMatrix * frontTransform.matrix());
    //    _selectors.emplace_back(new TerrainQuadNodeSelector(_frontTree));
    //
    //    dm::Transform backTransform;
    //    backTransform.rotateDegrees(180, glm::vec3(1, 0, 0));
    //    backTransform.translate(glm::vec3(0, 0, -radius / 2.f));
    //    _backTree = std::make_shared<TerrainQuadTree>(radius, deformation, rootMatrix * backTransform.matrix());
    //    _selectors.emplace_back(new TerrainQuadNodeSelector(_backTree));
    //
    //    dm::Transform leftTransform;
    //    leftTransform.rotateDegrees(-90, glm::vec3(0, 1, 0));
    //    leftTransform.translate(glm::vec3(-radius / 2.f, 0, 0));
    //    _leftTree = std::make_shared<TerrainQuadTree>(radius, deformation, rootMatrix * leftTransform.matrix());
    //    _selectors.emplace_back(new TerrainQuadNodeSelector(_leftTree));
    //
    //    dm::Transform rightTransform;
    //    rightTransform.rotateDegrees(90, glm::vec3(0, 1, 0));
    //    rightTransform.translate(glm::vec3(radius / 2.f, 0, 0));
    //    _rightTree = std::make_shared<TerrainQuadTree>(radius, deformation, rootMatrix * rightTransform.matrix());
    //    _selectors.emplace_back(new TerrainQuadNodeSelector(_rightTree));

    residualsProducer.reset(new CPUElevationDataTileProducer({resolution, resolution}));
    normalsProducer.reset(new CPUNormalDataTileProducer(residualsProducer.get()));

    _layers.elevation.producer.reset(new ElevationDataTileProducer(device(), residualsProducer.get()));
    _layers.normals.producer.reset(new NormalDataTileProducer(device(), normalsProducer.get()));
    _layers.elevation.renderer.reset(new TerrainElevationLayerRenderer(_layers.elevation.producer.get(), _layers.normals.producer.get()));

    _tileProducers.push_back(residualsProducer.get());
    _tileProducers.push_back(normalsProducer.get());
    _tileProducers.push_back(_layers.elevation.producer.get());
    _tileProducers.push_back(_layers.normals.producer.get());
    _layerRenderers.push_back(_layers.elevation.renderer.get());

    _terrainCache.push_back(_topTree.get());
    dg_assert_nm(_topTree->terrainId() == _terrainCache.size() - 1);

    for (TerrainLayerRenderer* layerRenderer : _layerRenderers) {
        layerRenderer->Init(device(), services());
    }
}

void TerrainRenderer::Submit(RenderQueue* renderQueue, const FrameView* view) {
    _nodesInScene.clear();

    for (std::unique_ptr<TerrainQuadNodeSelector>& selector : _selectors) {
        selector->SelectQuadNodes(view, &_nodesInScene);
    }

    _keysEntering.clear();
    _keysLeaving.clear();
    _keysInPrevScene = std::move(_keysInScene);

    for (const TerrainQuadNode* quad : _nodesInScene) {
        _keysInScene.insert(quad->key);
    }

    std::set_difference(begin(_keysInPrevScene), end(_keysInPrevScene), begin(_keysInScene), end(_keysInScene), std::inserter(_keysLeaving, begin(_keysLeaving)));
    std::set_difference(begin(_keysInScene), end(_keysInScene), begin(_keysInPrevScene), end(_keysInPrevScene), std::inserter(_keysEntering, begin(_keysEntering)));

    for (const TerrainQuadNode* quad : _nodesInScene) {
        dm::Rect3Dd worldRect = quad->worldRect();
        services()->debugDraw()->AddRect3D(worldRect, dutil::getColor(quad->key.lod), false);
    }

    for (DataTileProducer* producer : _tileProducers) {
        producer->Update(_nodesInScene, _keysLeaving, _keysEntering);
    }

    for (TerrainLayerRenderer* layer : _layerRenderers) {
        layer->Submit(renderQueue, view, _nodesInScene);
    }
}
