#include "KeyboardManager.h"

#define MapKeyboardKey(x) AddActionMapping("##x", InputCode::INPUT_##x, ActionConfig(false));
/*
MapKeyboardKey(KEY_A);
MapKeyboardKey(KEY_B);
MapKeyboardKey(KEY_C);
MapKeyboardKey(KEY_D);
MapKeyboardKey(KEY_E);
MapKeyboardKey(KEY_F);
MapKeyboardKey(KEY_G);
MapKeyboardKey(KEY_H);
MapKeyboardKey(KEY_I);
MapKeyboardKey(KEY_J);
MapKeyboardKey(KEY_K);
MapKeyboardKey(KEY_L);
MapKeyboardKey(KEY_M);
MapKeyboardKey(KEY_N);
MapKeyboardKey(KEY_O);
MapKeyboardKey(KEY_P);
MapKeyboardKey(KEY_Q);
MapKeyboardKey(KEY_R);
MapKeyboardKey(KEY_S);
MapKeyboardKey(KEY_T);
MapKeyboardKey(KEY_U);
MapKeyboardKey(KEY_V);
MapKeyboardKey(KEY_W);
MapKeyboardKey(KEY_X);
MapKeyboardKey(KEY_Y);
MapKeyboardKey(KEY_Z);
MapKeyboardKey(KEY_0);
MapKeyboardKey(KEY_1);
MapKeyboardKey(KEY_2);
MapKeyboardKey(KEY_3);
MapKeyboardKey(KEY_4);
MapKeyboardKey(KEY_5);
MapKeyboardKey(KEY_6);
MapKeyboardKey(KEY_7);
MapKeyboardKey(KEY_8);
MapKeyboardKey(KEY_9);
MapKeyboardKey(KEY_LEFT_SHIFT);
MapKeyboardKey(KEY_RIGHT_SHIFT);
MapKeyboardKey(KEY_TILDE);
MapKeyboardKey(KEY_CAPSLOCK);
MapKeyboardKey(KEY_BACKSPACE);*/
#undef MapKeyboardKey


namespace input {
    void KeyboardManager::HandleKeyPress(InputCode inputCode, int pressed) {
        if (!pressed) return;
        switch (inputCode) {
        case InputCode::INPUT_KEY_D:
            m_contents += "d";
            break;
        case InputCode::INPUT_KEY_S:
            m_contents += "s";
            break;
        case InputCode::INPUT_KEY_A:
            m_contents += "a";
            break;
        case InputCode::INPUT_KEY_W:
            m_contents += "w";
            break;
        case InputCode::INPUT_KEY_BACKSPACE:
            m_contents = m_contents.substr(0, m_contents.length() - 1); 
            break;
        default:
            return;
        }
    }

    bool KeyboardManager::HandlingKey(InputCode inputCode) {
        switch (inputCode) {
        case InputCode::INPUT_KEY_TILDE:
        case InputCode::INPUT_KEY_ESCAPE:
            return false;
        default:return m_isCapturing;
        }
    }

    void KeyboardManager::RestartCapture(std::string initial) {
        m_isCapturing = true;
        m_contents = initial;
    }

    void KeyboardManager::StopCapture() {
        m_isCapturing = false;
    }

    std::string KeyboardManager::GetText() {
        return m_contents;
    }
}