#pragma once
#include "InputCodes.h"
#include <string>
// This manages edit boxes, copy paste, etc.

namespace input {
    class KeyboardManager {
    private:
        std::string m_contents;
        bool m_isCapturing;
    public:
        KeyboardManager() {};
        void HandleKeyPress(InputCode inputCode, int pressed);
        bool HandlingKey(InputCode inputCode);
        void KeyboardManager::RestartCapture(std::string initial);
        void StopCapture();
        std::string GetText();
    };
}