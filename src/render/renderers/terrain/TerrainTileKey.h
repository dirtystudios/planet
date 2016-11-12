#pragma once

#include <cmath>
#include <stdint.h>
#include <string>
#include "Hash.h"
#
#include "DGAssert.h"
#include "DMath.h"
#include "Rectangle.h"

using TerrainId = uint32_t;

struct TerrainTileKey {
    TerrainId tid{0};
    uint32_t  tx{0};
    uint32_t  ty{0};
    uint32_t  lod{0};

    TerrainTileKey() {}
    TerrainTileKey(TerrainId tid, uint32_t tx, uint32_t ty, uint32_t lod) : tid(tid), tx(tx), ty(ty), lod(lod) {}

    size_t hash() const { return HashCombine(tid, tx, ty, lod); };

    std::array<TerrainTileKey, 4> subdivide() const {
        return {
            {
                TerrainTileKey(tid, tx * 2, ty * 2, lod + 1),         // bl
                TerrainTileKey(tid, tx * 2 + 1, ty * 2, lod + 1),     // br
                TerrainTileKey(tid, tx * 2, ty * 2 + 1, lod + 1),     // tl
                TerrainTileKey(tid, tx * 2 + 1, ty * 2 + 1, lod + 1), // tr
            },
        };
    }

    //    bool operator<(const TerrainTileKey& rhs) const;
    //    bool operator==(const TerrainTileKey& rhs) const;
};

//   _____ _____
//  |     |     |
//  | 1,0 | 1,1 |
//  |_____|_____|
//  |     |     |
//  | 0,0 | 1,0 |
//  |_____|_____|
bool operator<(const TerrainTileKey& lhs, const TerrainTileKey& rhs);

static TerrainTileKey keyToLOD(const TerrainTileKey& key, int32_t newLOD) {
    assert(key.lod != newLOD);

    int    diff    = key.lod - newLOD;
    double divisor = dm::pow(2, diff);
    return TerrainTileKey(key.tid, std::floor<uint32_t>(key.tx / divisor), std::floor<uint32_t>(key.ty / divisor), newLOD);
}

static TerrainTileKey getParentKey(const TerrainTileKey& key) {
    if (key.lod == 0)
        return key;

    TerrainTileKey parent(key.tid, std::floor<uint32_t>(key.tx / 2.f), std::floor<uint32_t>(key.ty / 2.f), key.lod - 1);

    return keyToLOD(key, key.lod - 1);
}

static dm::Rect2Dd localRectForKey(const TerrainTileKey& key, double rootTileSize) {
    double     size = rootTileSize / dm::pow(2.0, key.lod);
    glm::dvec2 bl(key.tx * size - rootTileSize / 2.0, key.ty * size - rootTileSize / 2.0);
    glm::dvec2 tr = bl + glm::dvec2(size, size);

    return dm::Rect2Dd(bl, tr);
}

static std::string asFilename(const TerrainTileKey& key) {
    return "tid_" + std::to_string(key.tid) + "-tx_" + std::to_string(key.tx) + "-ty_" + std::to_string(key.ty) + "-lod_" + std::to_string(key.lod);
}

static std::string toString(const TerrainTileKey& key) {
    return "TerrainTileKey [tid:" + std::to_string(key.tid) + " tx:" + std::to_string(key.tx) + " ty:" + std::to_string(key.ty) + " lod:" + std::to_string(key.lod) + "]";
}

bool operator==(const TerrainTileKey& lhs, const TerrainTileKey& rhs);
bool operator!=(const TerrainTileKey& lhs, const TerrainTileKey& rhs);

namespace std {
template <>
struct hash<TerrainTileKey> {
    typedef TerrainTileKey argument_type;
    typedef std::size_t    result_type;
    result_type operator()(argument_type const& s) const { return s.hash(); }
};
}
