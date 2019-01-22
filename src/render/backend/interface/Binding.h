#pragma once

#include <stdint.h>
#include "ResourceTypes.h"
#include "ShaderStageFlags.h"

namespace gfx {
struct Binding {
    enum class Type : uint8_t { ConstantBuffer, Buffer, Texture };

    Type type{Type::ConstantBuffer};
    ResourceId resource{0};
    uint32_t slot{0};
    ShaderStageFlags stageFlags{ShaderStageFlags::None};
};
}
