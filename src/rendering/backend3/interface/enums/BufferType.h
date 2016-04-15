#pragma once

#include <stdint.h>

namespace graphics {
enum class BufferType : uint32_t {
    VertexBuffer = 0,
    IndexBuffer,
    Count,
};
}