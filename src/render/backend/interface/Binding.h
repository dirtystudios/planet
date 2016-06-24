#pragma once

#include <stdint.h>
#include "ResourceTypes.h"

namespace gfx {
struct Binding {
    enum class Type : uint8_t {
        ConstantBuffer,
        Texture
    };

    Type type{ Type::ConstantBuffer };
    ResourceId resource{ 0 };
    uint32_t slot{ 0 };

};
}
