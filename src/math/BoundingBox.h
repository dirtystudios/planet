#pragma once

#include <array>
#include "glm/glm.hpp"

namespace dm {

struct BoundingBox {
    BoundingBox(glm::vec3 min, glm::vec3 max) : min(min), max(max){};
    BoundingBox(){};

    glm::vec3 min;
    glm::vec3 max;

    double distance(const glm::vec3& p) const {
        glm::vec3 point_on_bbox = glm::vec3();
        point_on_bbox.x         = (p.x < min.x) ? min.x : (p.x > max.x) ? max.x : p.x;
        point_on_bbox.y         = (p.y < min.y) ? min.y : (p.y > max.y) ? max.y : p.y;
        point_on_bbox.z         = (p.z < min.z) ? min.z : ((p.z > max.z) ? max.z : p.z);
        return glm::length(p - point_on_bbox);
    }

    std::array<glm::vec3, 8> corners() const {
        return {{min, glm::vec3(min.x, min.y, max.z), glm::vec3(min.x, max.y, min.z), glm::vec3(max.x, min.y, min.z), glm::vec3(max.x, max.y, min.z),
                 glm::vec3(max.x, min.y, max.z), glm::vec3(min.x, max.y, max.z), max}};
    }
};
}
