#pragma once

#include <string>
#include <vector>
#include <memory>
#include "MaterialData.h"

struct Material {
    Material(std::vector<MaterialData>&& datas) {
        matData = std::move(datas);
    }
    std::vector<MaterialData> matData;
};

using MaterialPtr = std::shared_ptr<Material>;
