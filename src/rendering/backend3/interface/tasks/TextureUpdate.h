#pragma once

#include "ResourceTypes.h"

namespace graphics {
struct TextureUpdate {
    TextureId textureId{0};
    void* data{nullptr};
    size_t len{0};
};
}