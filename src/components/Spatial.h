#pragma once

#include <glm/glm.hpp>
#include "Component.h"
#include "ComponentType.h"

class Spatial : public Component {
public:
    glm::dvec3 pos;
    glm::dvec3 velocity;
    glm::dvec3 direction;

    static constexpr ComponentType type() { return ComponentType::Spatial; }
};
