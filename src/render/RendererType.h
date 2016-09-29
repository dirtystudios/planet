#pragma once

#include <functional>
#include <stdint.h>
#include <string>

enum class RendererType : uint8_t {
    Skybox,
    ChunkedTerrain,
    Ui,
    Text,
    Mesh,
    Debug,
};

static std::string ToString(RendererType type) {
    switch (type) {
        case RendererType::Mesh:
            return "Mesh";
        case RendererType::ChunkedTerrain:
            return "Terrain";
        case RendererType::Ui:
            return "Ui";
        case RendererType::Text:
            return "Text";
        case RendererType::Skybox:
            return "Sky";
        default:
            return "unknown";
    }
    return "";
}

namespace std {
template <>
struct hash<RendererType> {
    size_t operator()(const RendererType& x) const { return std::hash<std::underlying_type<RendererType>::type>()(static_cast<std::underlying_type<RendererType>::type>(x)); }
};
}
