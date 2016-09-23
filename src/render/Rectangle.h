#pragma once
#include <glm/glm.hpp>

struct Rectangle {
    glm::vec2 bl{ 0, 0 };
    glm::vec2 tr{ 0, 0 };

    float GetWidth() const { return std::abs(tr.x - bl.x); }
    float GetHeight() const { return std::abs(tr.y - bl.y); }
    glm::vec2 TopLeft() const { return glm::vec2(bl.x, tr.y); };
    glm::vec2 BottomRight() const { return glm::vec2(tr.x, bl.y); };
};
