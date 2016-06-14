#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "ResourceTypes.h"

struct Material {
    graphics::ShaderParamId Kd{ 0 };
    graphics::ShaderParamId Ka{ 0 };
    graphics::ShaderParamId Ks{ 0 };
    graphics::ShaderParamId Ke{ 0 };
    graphics::ShaderParamId Ns{ 0 };

    std::vector<graphics::TextureId> diffuseTextures;

    // delete me when cBuffer support
    glm::vec3 KdData; // diffuse
    glm::vec3 KaData; // ambient
    glm::vec3 KsData; // specular
    glm::vec3 KeData; // emission
    float NsData;
};