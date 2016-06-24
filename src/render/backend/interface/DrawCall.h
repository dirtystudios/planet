#pragma once

#include <stdint.h>

namespace gfx {
struct DrawCall {
    enum class Type : uint8_t {
        Indexed = 0,
        Arrays
    };

    Type type{ Type::Indexed };
    uint32_t primitiveCount{ 0 };
    uint32_t offset{ 0 };
};
}
