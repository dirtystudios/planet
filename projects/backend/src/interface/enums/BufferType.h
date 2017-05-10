#pragma once

#include <stdint.h>

namespace gfx {
enum class BufferType : uint32_t {
    VertexBuffer = 0,
    IndexBuffer,
    ConstantBuffer,
    Count,
};
}
