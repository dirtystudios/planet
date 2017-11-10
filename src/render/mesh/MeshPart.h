#pragma once

#include "MeshGeometryData.h"
#include <string>
#include <vector>

struct MeshPart {
    MeshPart() {}
    MeshPart(uint32_t matIdx, uint32_t meshIdx)
    : matIdx(matIdx)
    , meshIdx(meshIdx)
    {}
    
    uint32_t matIdx{0};
    uint32_t meshIdx{0};
};
