#pragma once

#include <vector>
#include "RenderServiceLocator.h"
#include "TerrainDataTile.h"
#include "TerrainLayerType.h"

class TerrainLayerRenderer {
private:
    RenderServiceLocator* _services{nullptr};
    TerrainLayerType      _layerType;

public:
    TerrainLayerRenderer(TerrainLayerType layerType) : _layerType(layerType) {}

    TerrainLayerType LayerType() const { return _layerType; }

    void Init(RenderServiceLocator* services) {
        _services = services;
        OnInit();
    }

    virtual void OnInit(){};
    virtual void Submit(const std::vector<TerrainDataTile*>& selected) = 0;

protected:
    RenderServiceLocator* services() { return _services; }
};
