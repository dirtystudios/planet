#include "TerrainTileKey.h"
#include <string.h>

bool TerrainTileKey::operator<(const TerrainTileKey& rhs) { return memcmp(this, &rhs, sizeof(TerrainTileKey)) < 0; }
bool TerrainTileKey::operator==(const TerrainTileKey& rhs) { return memcmp(this, &rhs, sizeof(TerrainTileKey)) == 0; }
bool operator==(const TerrainTileKey& lhs, const TerrainTileKey& rhs) { return memcmp(&lhs, &rhs, sizeof(TerrainTileKey)) == 0; }
