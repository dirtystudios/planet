#pragma once

#include <stdint.h>
#include <stdlib.h>

namespace graphics {
    enum class ParamType {
        Int32,
        Float,
        Float2,
        Float3,
        Float4,
        Float4x4,
        Count
    };

    static size_t SizeofParam(ParamType type) {
        switch(type) {
            case ParamType::Int32: {
                return sizeof(int32_t);
            }
            case ParamType::Float: {
                return sizeof(float);
            }
            case ParamType::Float2: {
                return sizeof(float) * 2;
            }
            case ParamType::Float3: {
                return sizeof(float) * 3;
            }
            case ParamType::Float4: {
                return sizeof(float) * 4;
            }
            case ParamType::Float4x4: {
                return sizeof(float) * 16;
            }
            default: {
                return 0;
            }
        }
    }
}
