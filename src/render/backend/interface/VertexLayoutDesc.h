#pragma once

#include <cstring>
#include <functional>
#include <map>
#include <vector>
#include "DGAssert.h"
#include "Hash.h"
#include "ParamType.h"

namespace gfx {
enum class VertexAttributeType : uint8_t { Float = 0, Float2, Float3, Float4, Count };

enum class VertexAttributeStorage : uint8_t {
    UInt8N = 0,
    UInt16N,
    Float,
};

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
    VertexLayoutElement(VertexAttributeType type, VertexAttributeUsage usage, VertexAttributeStorage storage) : type(type), usage(usage), storage(storage) {}
    VertexLayoutElement(){};

    VertexAttributeType    type{VertexAttributeType::Float3};
    VertexAttributeUsage   usage{VertexAttributeUsage::Position};
    VertexAttributeStorage storage{VertexAttributeStorage::Float};
};

static size_t GetByteCount(VertexAttributeType type, VertexAttributeStorage storage) {
    size_t bytes = 0;
    switch (storage) {
        case VertexAttributeStorage::UInt8N: {
            bytes = sizeof(uint8_t);
            break;
        }
        case VertexAttributeStorage::UInt16N: {
            bytes = sizeof(uint16_t);
            break;
        }
        case VertexAttributeStorage::Float: {
            bytes = sizeof(float);
            break;
        }
        default:
            dg_assert_fail_nm();
    }

    switch (type) {
        case VertexAttributeType::Float:
            return bytes;
        case VertexAttributeType::Float2:
            return bytes * 2;
        case VertexAttributeType::Float3:
            return bytes * 3;
        case VertexAttributeType::Float4:
            return bytes * 4;
        default:
            dg_assert_fail_nm();
    }
}

static size_t GetByteCount(const VertexLayoutElement& attribute) { return GetByteCount(attribute.type, attribute.storage); }

struct VertexLayoutDesc {
    std::vector<VertexLayoutElement> elements;
    size_t                           stride() {
        size_t vertexStride = 0;
        for (VertexLayoutElement& element : elements) {
            vertexStride += GetByteCount(element);
        }
        return vertexStride;
    }
};

} // namespace

namespace std {
template <>
struct hash<gfx::VertexLayoutElement> {
    size_t operator()(const gfx::VertexLayoutElement& x) const {
        size_t key = 0;
        HashCombine(key, x.type);
        HashCombine(key, x.usage);
        return key;
    }
};

template <>
struct hash<gfx::VertexLayoutDesc> {
    size_t operator()(const gfx::VertexLayoutDesc& x) const {
        size_t key = 0;
        for (const gfx::VertexLayoutElement& e : x.elements) {
            HashCombine(key, e);
        }
        return key;
    }
};
}
