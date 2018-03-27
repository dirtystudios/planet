#pragma once

#include <stdint.h>
#include <functional>

enum class ComponentType : uint8_t {
    ChunkedTerrain = 0,
    Spatial,
    Skybox,
    UI,
    SkinnedMesh,
    Animation,
};

namespace std {
    template <> 
    struct hash<ComponentType> {
        size_t operator()(const ComponentType &x) const {
            return std::hash<std::underlying_type<ComponentType>::type>()(static_cast<std::underlying_type<ComponentType>::type>(x));
        }
    };
}
