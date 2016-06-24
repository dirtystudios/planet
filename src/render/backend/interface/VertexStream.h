#pragma once

#include "ResourceTypes.h"
#include <stdint.h>

namespace gfx {
struct VertexStream {
    BufferId vertexBuffer;
    uint32_t stride;
    uint32_t offset;
};
}
