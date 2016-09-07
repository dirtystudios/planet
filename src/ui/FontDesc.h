#pragma once

namespace ui {
    struct FontDesc {
        float textSize;   // ignored
        float color[3] = { 1.f, 1.f, 1.f }; // rgb, null = default
        // todo: stuff like...
        // std::string fontName;
        // bool bold? etc
    };
}