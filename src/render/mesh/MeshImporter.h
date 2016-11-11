#pragma once 

#include <vector>
#include "MeshPart.h"
#include <string>

namespace meshImport {
std::vector<MeshPart> LoadMeshDataFromFile(const std::string& fpath);
}