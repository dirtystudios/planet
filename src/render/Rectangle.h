#pragma once
#include <glm/glm.hpp>

struct Rectangle {
    glm::vec2 bl{ 0, 0 };
    glm::vec2 tr{ 0, 0 };

    float GetWidth() { return tr.x - bl.x; }
    float GetHeight() { return tr.y - bl.y; }
};