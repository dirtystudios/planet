#pragma once

#include <string>
#include "MeshGeometryData.h"

struct MeshPart {
    MeshPart() {}
    
    uint32_t matIdx;
    MeshGeometryData geometryData;
};