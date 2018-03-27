#pragma once

#include <glm/glm.hpp>
#include <string>
#include "Image.h"

enum class ShadingModel {
    None = 0,
    Flat,
    Gouraud,
    Phong,
    Blinn,
    Toon,
    OrenNayar,
    Minnaert,
    CookTorrance,
    Fresnel,
};

struct MaterialData {
    std::string name;
    ShadingModel shadingModel;
    glm::vec3 kd;
    glm::vec3 ka;
    glm::vec3 ke;
    glm::vec3 ks;
    float ns;
    std::string diffuseMap;
    dimg::Image diffuseData;
    std::string specularMap;
    
};
