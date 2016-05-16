#pragma once

#include "ParamType.h"
#include "Helpers.h"
#include <functional>
#include <vector>
#include <cstring>
#include <map>

namespace graphics {
enum class VertexAttributeType : uint8_t { Float = 0, Float2, Float3, Float4, Count };

enum class VertexAttributeUsage : uint8_t {
    Position = 0,
    Normal,
    Color0,
    Texcoord0,
    Count,
};

static string VertexAttributeUsageToString(VertexAttributeUsage usage) {
    switch (usage) {
    case VertexAttributeUsage::Position:
        return "POSITION";
    case VertexAttributeUsage::Normal:
        return "NORMAL";
    case VertexAttributeUsage::Color0:
        return "COLOR";
    case VertexAttributeUsage::Texcoord0:
        return "TEXCOORD";
    }
}

struct VertexLayoutElement {
    VertexAttributeType type;
    VertexAttributeUsage usage;
};

struct VertexLayoutDesc {
    std::vector<VertexLayoutElement> elements;
};

static size_t GetByteCount(VertexAttributeType paramType) {
    switch (paramType) {
    case VertexAttributeType::Float:
        return sizeof(float);
    case VertexAttributeType::Float2:
        return sizeof(float) * 2;
    case VertexAttributeType::Float3:
        return sizeof(float) * 3;
    case VertexAttributeType::Float4:
        return sizeof(float) * 4;
    default:
        assert(false);
    }
}
}

namespace std {
template <> struct hash<graphics::VertexAttributeType> {
    size_t operator()(const graphics::VertexAttributeType& x) const {
        return std::hash<std::underlying_type<graphics::VertexAttributeType>::type>()(
            static_cast<std::underlying_type<graphics::VertexAttributeType>::type>(x));
    }
};

template <> struct hash<graphics::VertexAttributeUsage> {
    size_t operator()(const graphics::VertexAttributeUsage& x) const {
        return std::hash<std::underlying_type<graphics::VertexAttributeUsage>::type>()(
            static_cast<std::underlying_type<graphics::VertexAttributeUsage>::type>(x));
    }
};

template <> struct hash<graphics::VertexLayoutElement> {
    size_t operator()(const graphics::VertexLayoutElement& x) const {
        size_t key = 0;
        HashCombine(key, x.type);
        HashCombine(key, x.usage);
        return key;
    }
};

template <> struct hash<graphics::VertexLayoutDesc> {
    size_t operator()(const graphics::VertexLayoutDesc& x) const {
        size_t key = 0;
        for (const graphics::VertexLayoutElement& e : x.elements) {
            HashCombine(key, e);
        }
        return key;
    }
};
}
