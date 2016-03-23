#include "KeyboardManager.h"

namespace input {
    KeyboardManager::KeyboardManager() {
        for (int x = 0; x < (uint32_t)InputCode::COUNT; ++x) {
            keyHoldTimer.emplace_back(0);
        }
        isLShift = isRShift = isLCtrl = isRCtrl = isLAlt = isRAlt = false;
    }

    // This just accounts for key repeating
    bool KeyboardManager::shouldKeyTrigger(InputCode inputCode, int pressed, float ms) {
        // weed out keys we don't want repeated
        if (handleModifier(inputCode, pressed)) return false;

        if (pressed) {
            if (keyHoldTimer[(int)inputCode] == 0) {
                keyHoldTimer[(int)inputCode] += ms;
                m_hasChanged = true;
                return true;
            }
            keyHoldTimer[(int)inputCode] += ms;
            if (keyHoldTimer[(int)inputCode] < m_repeatDelay) {
                return false;
            }
            float temp = keyHoldTimer[(int)inputCode] - m_repeatDelay;
            if (temp > m_repeatInterval) {
                keyHoldTimer[(int)inputCode] = m_repeatDelay;
                m_hasChanged = true;
                return true;
            }
        }
        else {
            keyHoldTimer[(int)inputCode] = 0;
        }
        return false;
    }

    void KeyboardManager::HandleKeyPress(InputCode inputCode, int pressed, float ms) {
        if (!shouldKeyTrigger(inputCode, pressed, ms)) return;

        if (isLShift || isRShift) {
            if (handleKeyOutput(inputCode, uppercaseKeyString, InputCode::INPUT_KEY_A, InputCode::INPUT_KEY_Z)) return;
            if (handleKeyOutput(inputCode, shiftedSymbolKeyString, InputCode::INPUT_KEY_BACKTICK, InputCode::INPUT_KEY_SPACE)) return;
            if (handleKeyOutput(inputCode, numbericSymbolsString, InputCode::INPUT_KEY_0, InputCode::INPUT_KEY_9)) return;
        }
        else {
            if (handleKeyOutput(inputCode, lowercaseKeyString, InputCode::INPUT_KEY_A, InputCode::INPUT_KEY_Z)) return;
            if (handleKeyOutput(inputCode, symbolKeyString, InputCode::INPUT_KEY_BACKTICK, InputCode::INPUT_KEY_SPACE)) return;
            if (handleKeyOutput(inputCode, numericKeyString, InputCode::INPUT_KEY_0, InputCode::INPUT_KEY_9)) return;
        }

        if (handleCursorKeys(inputCode)) return;
        if (handleDeleteKeys(inputCode)) return;
    }

    // This function notifys the input manager if we want to 'take-care' of the key handling
    bool KeyboardManager::HandlingKey(InputCode inputCode) {
        // Only Keyboard keys
        if (inputCode > InputCode::INPUT_KEY_DOWN) return false;

        // Avoid keys that ui manager will want instead of us
        switch (inputCode) {
        case InputCode::INPUT_KEY_TAB:
        case InputCode::INPUT_KEY_ENTER:
        case InputCode::INPUT_KEY_ESCAPE:
        case InputCode::INPUT_KEY_BACKTICK:
            return false;
        default: return m_isCapturing;
        }
    }

    bool KeyboardManager::handleDeleteKeys(InputCode inputCode) {
        // One-off deletions
        if (inputCode == InputCode::INPUT_KEY_BACKSPACE) {
            if (m_cursorPos != 0) {
                std::string first = m_contents.substr(0, m_cursorPos - 1);
                std::string second = m_contents.substr(m_cursorPos, m_contents.length() - m_cursorPos);
                m_contents = first + second;
                --m_cursorPos;
            }
            return true;
        }
        else if (inputCode == InputCode::INPUT_KEY_DELETE) {
            if (m_cursorPos != m_contents.length()) {
                std::string first = m_contents.substr(0, m_cursorPos);
                std::string second = m_contents.substr(m_cursorPos + 1, m_contents.length() - (m_cursorPos + 1));
                m_contents = first + second;
            }
            return true;
        }
        return false;
    }

    bool KeyboardManager::handleCursorKeys(InputCode inputCode) {
        if (inputCode == InputCode::INPUT_KEY_LEFT) {
            if (m_cursorPos != 0) --m_cursorPos;
        }
        else if (inputCode == InputCode::INPUT_KEY_RIGHT) {
            if (m_cursorPos < m_contents.length()) ++m_cursorPos;
        }
        else if (inputCode == InputCode::INPUT_KEY_HOME) {
            m_cursorPos = 0;
        }
        else if (inputCode == InputCode::INPUT_KEY_END) {
            m_cursorPos = m_contents.length();
        }
        else {
            return false;
        }
        return true;
    }

    // This function takes a range, spits the key into the string
    // Takes into account cursor position and cursor moving
    bool KeyboardManager::handleKeyOutput(InputCode inputCode, const char *keyString, InputCode lowRange, InputCode highRange) {
        if (!(inputCode >= lowRange && inputCode <= highRange))
            return false;
        int temp = (int)inputCode - (int)lowRange;

        m_contents = m_contents.insert(m_cursorPos, 1, keyString[temp]);
        ++m_cursorPos;
        
        return true;
    }

    bool KeyboardManager::handleModifier(InputCode inputCode, int pressed) {
        if (!(inputCode >= InputCode::INPUT_KEY_LEFT_SHIFT && inputCode <= InputCode::INPUT_KEY_WINDOWS))
            return false;
        
        if (inputCode == InputCode::INPUT_KEY_LEFT_SHIFT)
            isLShift = (pressed > 0);
        else if (inputCode == InputCode::INPUT_KEY_RIGHT_SHIFT)
            isRShift = (pressed > 0);
        else if (inputCode == InputCode::INPUT_KEY_LEFT_CTRL)
            isLCtrl = (pressed > 0);
        else if (inputCode == InputCode::INPUT_KEY_RIGHT_CTRL)
            isRCtrl = (pressed > 0);
        else if (inputCode == InputCode::INPUT_KEY_LEFT_ALT)
            isLAlt = (pressed > 0);
        else if (inputCode == InputCode::INPUT_KEY_RIGHT_ALT)
            isRAlt = (pressed > 0);

        return true;
    }

    int KeyboardManager::GetCursorPosition() {
        return m_cursorPos;
    }

    void KeyboardManager::RestartCapture(std::string initial, uint32_t cursorPosition) {
        m_hasChanged = true;
        m_isCapturing = true;
        m_contents = initial;
        m_cursorPos = cursorPosition;
        for (int x = 0; x < (uint32_t)InputCode::COUNT; ++x) {
            keyHoldTimer.emplace_back(0);
        }
    }

    bool KeyboardManager::TextHasChanged() {
        return m_hasChanged;
    }

    void KeyboardManager::StopCapture() {
        m_isCapturing = false;
    }

    std::string KeyboardManager::GetText() {
        m_hasChanged = false;
        return m_contents;
    }
}