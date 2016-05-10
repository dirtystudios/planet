#pragma once
#include "GPUTile.h"

struct Tile {
    Tile(uint32_t lod, uint32_t tx, uint32_t ty, GPUTile* data) : lod(lod), tx(tx), ty(ty), data(data){};

    uint32_t lod;
    uint32_t tx;
    uint32_t ty;

    GPUTile* data;
};