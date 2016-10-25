#pragma once

#include <glm/glm.hpp>

namespace ui {
    struct FontDesc {
        float textSize{ 12.f };   // ignored
        glm::vec3 color = { 1.f, 1.f, 1.f }; // rgb, null = default
        // todo: stuff like...
        // std::string fontName;
        // bool bold? etc
    };
}