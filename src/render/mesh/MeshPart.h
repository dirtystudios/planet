#pragma once

#include <string>
#include "MeshGeometryData.h"

struct MeshPart {
    MeshPart() {}
    MeshPart(uint32_t matIdx, MeshGeometryData&& geometryData)
    : matIdx(matIdx)
    , geometryData(geometryData)
    {}
    
    uint32_t matIdx{0};
    MeshGeometryData geometryData;
};
