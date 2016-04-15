#pragma once

#include <stdint.h>
namespace graphics {
enum class PrimitiveType : uint8_t {
    Triangles = 0,
    LineStrip = 1,
    Lines,
    Count,
};
}