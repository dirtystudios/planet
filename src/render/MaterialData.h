#pragma once

#include <glm/glm.hpp>
#include <string>

struct MaterialData {
    std::string name;
    int shadingModel;
    glm::vec3 kd;
    glm::vec3 ka;
    glm::vec3 ke;
    glm::vec3 ks;
    float ns;
    std::string diffuseMap;
    std::string specularMap;
    
};
