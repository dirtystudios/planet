#pragma once

#include "ChunkedTerrainNode.h"
#include "RenderObj.h"
#include "RendererType.h"
#include "ResourceTypes.h"

struct ChunkedTerrainRenderObj : public RenderObj {
    ChunkedTerrainRenderObj() : RenderObj(RendererType::ChunkedTerrain) {}

    ChunkedTerrainNode* root;
    float size;
    double x;
    double y;

    graphics::BufferId vertexBuffer;
    uint32_t vertexCount;

    std::function<double(double x, double y, double z)> heightmapGenerator;
};