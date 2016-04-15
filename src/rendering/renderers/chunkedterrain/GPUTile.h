#pragma once

struct GPUTile {
    GPUTile(){};
    GPUTile(graphics::TextureId textureId, uint32_t idx) : textureId(textureId), index(idx){};

    graphics::TextureId textureId;
    uint32_t index;
};
