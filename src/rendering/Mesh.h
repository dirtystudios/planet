#pragma once 

#include "ResourceTypes.h"

struct Mesh {    
	graphics::BufferId vertexBuffer {0};
	graphics::BufferId indexBuffer {0};
	uint32_t indexCount {0};
	uint32_t indexOffset {0};
    uint32_t vertexStride{0};
    uint32_t vertexOffset{0};
    
};