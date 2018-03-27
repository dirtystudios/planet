#pragma once

#include "MeshPart.h"
#include <string>
#include <vector>
#include <glm/glm.hpp>

struct MeshNode {
    glm::mat4 localTransform{ 0.f };
    std::string name;
    std::vector<MeshPart> meshParts;
};