#pragma once
#include "InputCodes.h"
#include <string>
#include <vector>
// This manages edit boxes, copy paste, etc.

namespace input {
    class KeyboardManager {
    private:
        const char *lowercaseKeyString = "abcdefghijklmnopqrstuvwxyz";
        const char *uppercaseKeyString = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        const char *numericKeyString = "0123456789";
        const char *symbolKeyString = "`-=[]\\;',./ ";
        const char *shiftedSymbolKeyString = "~_+{}|:\"<>?";
        const char *numbericSymbolsString = ")!@#$%^&*(";

        bool isLShift, isRShift, isLCtrl, isRCtrl, isLAlt, isRAlt;
        std::string m_contents;
        bool m_isCapturing;
        // Note: these are in milleseconds.
        const float m_repeatDelay = 500;
        const float m_repeatInterval = 30;
        std::vector<float> keyHoldTimer;
        uint32_t m_cursorPos;
        bool m_hasChanged;
    public:
        KeyboardManager();
        void HandleKeyPress(InputCode inputCode, int pressed, float dt);
        bool HandlingKey(InputCode inputCode);
        void KeyboardManager::RestartCapture(std::string initial, uint32_t cursorPosition);
        void StopCapture();
        std::string GetText();
        bool TextHasChanged();
        int GetCursorPosition();

    private:
        bool shouldKeyTrigger(InputCode inputCode, int pressed, float dt);
        bool handleModifier(InputCode inputCode, int pressed);
        bool handleKeyOutput(InputCode inputCode, const char *keyString, InputCode lowRange, InputCode highRange);
        bool handleCursorKeys(InputCode inputCode);
        bool handleDeleteKeys(InputCode inputCode);
    };
}