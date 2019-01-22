#pragma once

#include <functional>
#include <stdint.h>
#include <string>

enum class RendererType : uint8_t
{
    Skybox,
    Terrain,
    Ui,
    Text,
    Mesh,
    Debug,
    RayTrace,
};

static std::string ToString(RendererType type) {
    switch (type) {
        case RendererType::Mesh:
            return "Mesh";
        case RendererType::Terrain:
            return "Terrain";
        case RendererType::Ui:
            return "Ui";
        case RendererType::Text:
            return "Text";
        case RendererType::Skybox:
            return "Sky";
        case RendererType::Debug:
            return "Debug";
        case RendererType::RayTrace:
            return "RayTrace";
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
