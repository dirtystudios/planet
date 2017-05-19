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

TerrainRenderer::TerrainRenderer()  : TypedRenderer<TerrainRenderObj>(RendererType::Terrain) {}

TerrainRenderer::~TerrainRenderer() {}

// TODO: Layer abstraction in TerrainRenderer is no good. Think of something better.
// TODO: Producers have alot of duplicate code. Do something about it
// TODO: Quadnodes needs bounding boxes with proper elevation data
// TODO: Properly rendering partial tiles. (ex. Scale texure coords). Necessary to fix flickering
// TODO: producers need to walk tree instead of jumping straight to tile so that we have high lod fallback
// TODO: selection can select too many tiles, raping caches
// TODO: Borders for normalmaps

void TerrainRenderer::OnInit() {
    uint32_t resolution = 128;

    _producers.elevations.cpu.reset(new CPUElevationDataTileProducer({resolution, resolution}));
    _producers.elevations.gpu.reset(new ElevationDataTileProducer(device(), _producers.elevations.cpu.get()));
    _producers.normals.cpu.reset(new CPUNormalDataTileProducer(_producers.elevations.cpu.get()));
    _producers.normals.gpu.reset(new NormalDataTileProducer(device(), _producers.normals.cpu.get()));
    _renderers.baseLayer.reset(new TerrainElevationLayerRenderer(_producers.elevations.gpu.get(), _producers.normals.gpu.get()));

    _tileProducers.push_back(_producers.elevations.cpu.get());
    _tileProducers.push_back(_producers.elevations.gpu.get());
    _tileProducers.push_back(_producers.normals.cpu.get());
    _tileProducers.push_back(_producers.normals.gpu.get());
    _layerRenderers.push_back(_renderers.baseLayer.get());

    for (TerrainLayerRenderer* layerRenderer : _layerRenderers) {
        layerRenderer->Init(device(), services());
    }
}

void TerrainRenderer::Register(TerrainRenderObj* renderObj) { _renderObjs.push_back(renderObj); }

void TerrainRenderer::Submit(RenderQueue* renderQueue, const FrameView* view) {
    _nodesInScene.clear();

    for (const TerrainRenderObj* terrain : _renderObjs) {
        for (TerrainQuadTree* tree : terrain->getQuadTrees()) {
            TerrainQuadNodeSelector().SelectQuadNodes(view, tree, &_nodesInScene);
        }
    }

    _keysEntering.clear();
    _keysLeaving.clear();
    _keysInPrevScene = std::move(_keysInScene);

    for (const TerrainQuadNode* quad : _nodesInScene) {
        _keysInScene.insert(quad->key);
    }

    std::set_difference(begin(_keysInPrevScene), end(_keysInPrevScene), begin(_keysInScene), end(_keysInScene), std::inserter(_keysLeaving, begin(_keysLeaving)));
    std::set_difference(begin(_keysInScene), end(_keysInScene), begin(_keysInPrevScene), end(_keysInPrevScene), std::inserter(_keysEntering, begin(_keysEntering)));

    // draw nodes in 3D
    for (const TerrainQuadNode* quad : _nodesInScene) {
        // dm::Rect3Dd worldRect = quad->worldRect();
        // services()->debugDraw()->AddRect3D(worldRect, dutil::getColor(quad->key.lod), false);
    }

    // draw selected nodes on 2D ui element
    if (_nodesInScene.size() > 0) {
        dm::Rect3Dd root  = _nodesInScene[0]->terrain->localRectForKey({_nodesInScene[0]->key.tid, 0, 0, 0});
        float terrainSize = root.width();
        float xsize       = 250;
        float ysize       = 250.f;
        glm::vec4 bgColor = {0.85, 0.85, 0.85, 1.f};
        float border      = 1.f;
        float xloc        = view->viewport.topLeftXY[0] + xsize / 2.f;
        float yloc        = view->viewport.height - view->viewport.topLeftXY[1] - ysize / 2.f; // viewport param isnt actually top left

        glm::vec2 bl = {xloc + (float)root.bl().x / terrainSize * xsize, yloc + (float)root.bl().y / terrainSize * ysize};
        glm::vec2 tr = {xloc + (float)root.tr().x / terrainSize * xsize, yloc + (float)root.tr().y / terrainSize * ysize};
        dm::Rect2Df screenRect(bl, tr);
        services()->debugDraw()->AddRect2D(screenRect, bgColor, true);

        for (const TerrainQuadNode* quad : _nodesInScene) {
            glm::vec2 bl = {xloc + (float)quad->localRect.bl().x / terrainSize * xsize, yloc + (float)quad->localRect.bl().y / terrainSize * ysize};
            glm::vec2 tr = {xloc + (float)quad->localRect.tr().x / terrainSize * xsize, yloc + (float)quad->localRect.tr().y / terrainSize * ysize};
            dm::Rect2Df screenRect(bl + border / 2.f, tr - border / 2.f);

            services()->debugDraw()->AddRect2D(screenRect, glm::vec4{ dutil::getColor(quad->key.lod), 1.f }, true);
        }
    }

    for (DataTileProducer* producer : _tileProducers) {
        producer->Update(_nodesInScene, _keysLeaving, _keysEntering);
    }

    for (TerrainLayerRenderer* layer : _layerRenderers) {
        layer->Submit(renderQueue, view, _nodesInScene);
    }
}
