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

        // Note: these are in milleseconds.
        const float m_repeatDelay = 500;
        const float m_repeatInterval = 30;

        std::vector<float> keyHoldTimer;

        bool isRShift = false, isRCtrl = false, isRAlt = false;
        bool isLShift = false, isLCtrl = false, isLAlt = false;
        std::string m_contents = "";
        bool m_isCapturing = false;
        uint32_t m_cursorPos = 0;
        bool m_hasChanged = true;
    public:
        KeyboardManager();
        void HandleKeyPress(InputCode inputCode, int pressed, float dt);
        bool HandlingKey(InputCode inputCode);
        void RestartCapture(std::string initial, uint32_t cursorPosition);
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