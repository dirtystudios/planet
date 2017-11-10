#pragma once
#include <vector>
#include <string>

#include "MaterialData.h"

namespace materialImport {
    std::vector<MaterialData> LoadMaterialDataFromFile(const std::string& fpath);
}
