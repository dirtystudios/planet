#pragma once

// Implement this thing and give it to a frame if you want 'callbacks' to happen on events
// Not super thrilled with this way of doing things, but using it for a quick concept

namespace ui {
    // forward declare to avoid shitty circular dependencies
    class UIFrame;
    class EditBox;
    class ScriptHandler {
    public:
        virtual void OnEnterPressed(EditBox& frame) = 0;
    };
}