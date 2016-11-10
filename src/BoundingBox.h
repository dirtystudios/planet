#ifndef __bounding_box_h__
#define __bounding_box_h__

#include "glm/glm.hpp"

struct BoundingBox {
    BoundingBox(glm::vec3 min, glm::vec3 max) : min(min), max(max) {};
    BoundingBox() {};
    
    glm::vec3 min;
    glm::vec3 max;    

    double GetDistanceFromBoundingBox(const glm::vec3& p) const {
        glm::vec3 point_on_bbox = glm::vec3();    
        point_on_bbox.x = (p.x < min.x) ? min.x : (p.x > max.x) ? max.x : p.x;
        point_on_bbox.y = (p.y < min.y) ? min.y : (p.y > max.y) ? max.y : p.y;
        point_on_bbox.z = (p.z < min.z) ? min.z : ((p.z > max.z) ? max.z : p.z);    
        return glm::length(p - point_on_bbox);    
    }
};

#endif
