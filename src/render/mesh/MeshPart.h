#pragma once

#include <string>
#include "Material.h"
#include "MeshGeometryData.h"

struct MeshPart {
    MeshPart() {}
    
	std::string materialName{ 0 };
	MeshGeometryData geometryData;
};