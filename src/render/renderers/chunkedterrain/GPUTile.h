#pragma once

struct GPUTile {
    GPUTile(){};
    GPUTile(gfx::TextureId textureId, uint32_t idx) : textureId(textureId), index(idx){};

    gfx::TextureId textureId;
    uint32_t index;
};
