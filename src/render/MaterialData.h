#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include "Image.h"

struct MaterialData {
    uint8_t                      illumination{0};
    glm::vec3                    Kd; // diffuse
    glm::vec3                    Ka; // ambient
    glm::vec3                    Ks; // specular
    glm::vec3                    Ke; // emission
    float                        Ns;
    std::unique_ptr<dimg::Image> diffuseTexture;
};
