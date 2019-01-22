#pragma once

#include <stdint.h>

namespace gfx {
    struct DispatchCall {
        enum class Type : uint8_t {
            Direct = 0,
            Indirect
        };

        Type type{ Type::Direct };
        uint32_t groupX{ 0 };
        uint32_t groupY{ 0 };
        uint32_t groupZ{ 0 };
    };
}
