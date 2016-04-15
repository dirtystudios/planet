#pragma once

#include "BoundingBox.h"

struct ChunkedTerrainNode {
    uint32_t lod;
    uint32_t tx;
    uint32_t ty;

    double x;
    double y;

    BoundingBox bbox;
    float size;

    ChunkedTerrainNode* children = {NULL};
};