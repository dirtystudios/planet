#pragma once

namespace gfx {
enum class BufferLifetime : uint8_t {
    Persistent = 0,
    Transient  = 1, // lives for one frame
};
}
