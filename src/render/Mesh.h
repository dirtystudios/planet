#pragma once

#include "ResourceTypes.h"

struct Mesh {
    // TODO: streamline this to use vertex streams so it can generate vertex streams and draw call used in DrawItem
    // creation
    gfx::BufferId vertexBuffer{0};
    gfx::BufferId indexBuffer{0};
    uint32_t indexCount{0};
    uint32_t indexOffset{0};
    uint32_t vertexStride{0};
    uint32_t vertexOffset{0};
    uint32_t vertexCount{0};
};
