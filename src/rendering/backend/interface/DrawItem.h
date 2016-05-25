#pragma once

namespace graphics {
    struct DrawItem {
        size_t size() { return static_cast<size_t>(*(size_t*) (this)); }
    };
}
