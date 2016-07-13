#pragma once

#include "ParamType.h"
#include "Helpers.h"
#include <functional>
#include <vector>
#include <cstring>
#include <map>

namespace gfx {
enum class VertexAttributeType : uint8_t { Float = 0, Float2, Float3, Float4, Count };

enum class VertexAttributeUsage : uint8_t {
    Position = 0,
    Normal,
    Color0,
    Texcoord0,
};

static std::string VertexAttributeUsageToString(VertexAttributeUsage usage) {
    switch (usage) {
    case VertexAttributeUsage::Position:
        return "POSITION";
    case VertexAttributeUsage::Normal:
        return "NORMAL";
    case VertexAttributeUsage::Color0:
        return "COLOR";
    case VertexAttributeUsage::Texcoord0:
        return "TEXCOORD";
    default:
        assert(false);
    }
    return "";
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
        return 0;
    }
}
}

namespace std {
template <> struct hash<gfx::VertexAttributeType> {
    size_t operator()(const gfx::VertexAttributeType& x) const {
        return std::hash<std::underlying_type<gfx::VertexAttributeType>::type>()(
            static_cast<std::underlying_type<gfx::VertexAttributeType>::type>(x));
    }
};

template <> struct hash<gfx::VertexAttributeUsage> {
    size_t operator()(const gfx::VertexAttributeUsage& x) const {
        return std::hash<std::underlying_type<gfx::VertexAttributeUsage>::type>()(
            static_cast<std::underlying_type<gfx::VertexAttributeUsage>::type>(x));
    }
};

template <> struct hash<gfx::VertexLayoutElement> {
    size_t operator()(const gfx::VertexLayoutElement& x) const {
        size_t key = 0;
        HashCombine(key, x.type);
        HashCombine(key, x.usage);
        return key;
    }
};

template <> struct hash<gfx::VertexLayoutDesc> {
    size_t operator()(const gfx::VertexLayoutDesc& x) const {
        size_t key = 0;
        for (const gfx::VertexLayoutElement& e : x.elements) {
            HashCombine(key, e);
        }
        return key;
    }
};
}
