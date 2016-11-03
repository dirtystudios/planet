#pragma once

#include <glm/glm.hpp>
#include "Rectangle.h"

class DebugDrawInterface {
public:
    virtual void AddLine2D(const glm::vec2& start, const glm::vec2& end, glm::vec3 color) = 0;
    virtual void AddRect2D(const dm::Rect2Df& rect, const glm::vec3& color, bool filled = false) = 0;

    virtual void AddLine3D(const glm::vec3& start, const glm::vec3& end, glm::vec3 color) = 0;
    virtual void AddRect3D(const dm::Rect3Df& rect, const glm::vec3& color, bool filled = false) = 0;

    virtual void AddSphere3D(const glm::vec3& origin, float radius) = 0;
};
