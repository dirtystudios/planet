#pragma once 

#include <vector>
#include "IndexedMeshData.h"
#include <string>

namespace meshImport {
std::vector<IndexedMeshData> LoadMeshDataFromFile(const std::string& fpath);	
}