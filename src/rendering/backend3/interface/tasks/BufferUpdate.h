#pragma once

#include "ResourceTypes.h"

namespace graphics {
struct BufferUpdate {
    BufferId bufferId{0};
    void* data{nullptr};
    size_t len{0};
};
}