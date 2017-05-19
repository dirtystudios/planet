#pragma once

#include <glm/glm.hpp>
#include "Rectangle.h"

class DebugDrawInterface {
public:
    virtual void AddLine2D(const glm::vec2& start, const glm::vec2& end, const glm::vec4& color) = 0;
    virtual void AddRect2D(const dm::Rect2Df& rect, const glm::vec4& color, bool filled = false) = 0;

    virtual void AddCircle2D(const glm::vec2& origin, float r, const glm::vec4& color, bool filled = false) = 0;

    virtual void AddLine3D(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color) = 0;
    virtual void AddRect3D(const dm::Rect3Df& rect, const glm::vec4& color, bool filled = false) = 0;

    virtual void AddSphere3D(const glm::vec3& origin, float radius) = 0;
    virtual void AddText(const glm::vec2& start, const glm::vec4& color, const std::string& text) = 0;
};
