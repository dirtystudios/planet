#pragma once

#include <stdint.h>
#include <string>
#include "Hash.h"

struct TerrainTileKey {
    uint32_t tid{0};
    uint32_t tx{0};
    uint32_t ty{0};
    uint32_t lod{0};

    TerrainTileKey(){};

    TerrainTileKey(uint32_t tid, uint32_t tx, uint32_t ty, uint32_t lod) : tid(tid), tx(tx), ty(ty), lod(lod) {}

    size_t hash() const { return HashCombine(tid, tx, ty, lod); };

    bool operator<(const TerrainTileKey& lhs);
    bool operator==(const TerrainTileKey& lhs);
};

static std::string ToString(const TerrainTileKey& key) {
    return "TerrainTileKey [tid:" + std::to_string(key.tid) + " tx:" + std::to_string(key.tx) + " ty:" + std::to_string(key.ty) + " lod:" + std::to_string(key.lod) + "]";
}

bool operator==(const TerrainTileKey& lhs, const TerrainTileKey& rhs);

namespace std {
template <>
struct hash<TerrainTileKey> {
    typedef TerrainTileKey argument_type;
    typedef std::size_t    result_type;
    result_type operator()(argument_type const& s) const { return s.hash(); }
};
}
