#pragma once

#include "ShaderDataType.h"

namespace gfx {
struct ShaderData {
    ShaderDataType type{ShaderDataType::Source};
    const void* data{nullptr};
    size_t len{0};
};
}
