#pragma once

#include <vector>
#include "RenderQueue.h"
#include "RenderServiceLocator.h"
#include "RenderView.h"
#include "TerrainDataTile.h"
#include "TerrainLayerType.h"
#include "TerrainQuadNode.h"

class TerrainLayerRenderer {
private:
    RenderServiceLocator* _services{nullptr};
    gfx::RenderDevice*    _device{nullptr};
    TerrainLayerType      _layerType;

public:
    TerrainLayerRenderer(TerrainLayerType layerType) : _layerType(layerType) {}

    TerrainLayerType LayerType() const { return _layerType; }

    void Init(gfx::RenderDevice* device, RenderServiceLocator* services) {
        _device   = device;
        _services = services;
        OnInit();
    }

    virtual void OnInit(){};
    virtual void Submit(RenderQueue* renderQueue, RenderView* renderView, const std::vector<const TerrainQuadNode*>& selectedQuads) = 0;

protected:
    gfx::RenderDevice*    device() { return _device; }
    RenderServiceLocator* services() { return _services; }
};
