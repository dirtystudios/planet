#pragma once

#include <stdint.h>
#include <functional>

enum class RendererType : uint8_t {
    Skybox,
    ChunkedTerrain,
    Ui,
    Text,
    Mesh,
    Debug,
};

namespace std {
template <> struct hash<RendererType> {
    size_t operator()(const RendererType& x) const {
        return std::hash<std::underlying_type<RendererType>::type>()(
            static_cast<std::underlying_type<RendererType>::type>(x));
    }
    };
}
