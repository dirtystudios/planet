#pragma once
namespace input {
    enum class InputCode {
        // Keyboard

        // Letters
        INPUT_KEY_A = 0,
        INPUT_KEY_B,
        INPUT_KEY_C,
        INPUT_KEY_D,
        INPUT_KEY_E,
        INPUT_KEY_F,
        INPUT_KEY_G,
        INPUT_KEY_H,
        INPUT_KEY_I,
        INPUT_KEY_J,
        INPUT_KEY_K,
        INPUT_KEY_L,
        INPUT_KEY_M,
        INPUT_KEY_N,
        INPUT_KEY_O,
        INPUT_KEY_P,
        INPUT_KEY_Q,
        INPUT_KEY_R,
        INPUT_KEY_S,
        INPUT_KEY_T,
        INPUT_KEY_U,
        INPUT_KEY_V,
        INPUT_KEY_W,
        INPUT_KEY_X,
        INPUT_KEY_Y,
        INPUT_KEY_Z,

        // Numbers
        INPUT_KEY_0,
        INPUT_KEY_1,
        INPUT_KEY_2,
        INPUT_KEY_3,
        INPUT_KEY_4,
        INPUT_KEY_5,
        INPUT_KEY_6,
        INPUT_KEY_7,
        INPUT_KEY_8,
        INPUT_KEY_9,

        // Modifiers
        INPUT_KEY_LEFT_SHIFT,
        INPUT_KEY_RIGHT_SHIFT,
        INPUT_KEY_LEFT_CTRL,
        INPUT_KEY_RIGHT_CTRL,
        INPUT_KEY_LEFT_ALT,
        INPUT_KEY_RIGHT_ALT,
        INPUT_KEY_WINDOWS,

        // Utility
        INPUT_KEY_CAPSLOCK,
        INPUT_KEY_BACKSPACE,
        INPUT_KEY_INSERT,
        INPUT_KEY_HOME,
        INPUT_KEY_PAGEUP,
        INPUT_KEY_DELETE,
        INPUT_KEY_END,
        INPUT_KEY_PAGEDOWN,
        INPUT_KEY_TAB,
        INPUT_KEY_ESCAPE,
        INPUT_KEY_ENTER,

        // Symbols
        INPUT_KEY_BACKTICK,
        INPUT_KEY_HYPEN,
        INPUT_KEY_EQUALS,
        INPUT_KEY_LEFTSQUARE,
        INPUT_KEY_RIGHTSQUARE,
        INPUT_KEY_BACKSLASH,
        INPUT_KEY_SEMICOLON,
        INPUT_KEY_SINGLEQUOTE,
        INPUT_KEY_COMMA,
        INPUT_KEY_DOT,
        INPUT_KEY_FORWARDSLASH,

        INPUT_KEY_SPACE,

        // Direction
        INPUT_KEY_LEFT,
        INPUT_KEY_RIGHT,
        INPUT_KEY_UP,
        INPUT_KEY_DOWN,

        // Mouse Codes
        INPUT_MOUSE_KEY1,
        INPUT_MOUSE_KEY2,
        INPUT_MOUSE_KEY3,
        INPUT_MOUSE_KEY4,
        INPUT_MOUSE_WHEELDOWN,
        INPUT_MOUSE_WHEELUP,
        INPUT_MOUSE_XAXIS,
        INPUT_MOUSE_YAXIS,
        INPUT_MOUSE_XAXISRELATIVE,
        INPUT_MOUSE_YAXISRELATIVE,

        // Controller Codes
        INPUT_GAMEPAD_Y,
        INPUT_GAMEPAD_X,
        INPUT_GAMEPAD_A,
        INPUT_GAMEPAD_B,
        INPUT_GAMEPAD_START,
        INPUT_GAMEPAD_SELECT,
        INPUT_GAMEPAD_UP,
        INPUT_GAMEPAD_DOWN,
        INPUT_GAMEPAD_RIGHT,
        INPUT_GAMEPAD_LEFT,
        INPUT_GAMEPAD_RSHOULDER,
        INPUT_GAMEPAD_LSHOULDER,
        INPUT_GAMEPAD_RSTICKY,
        INPUT_GAMEPAD_RSTICKX,
        INPUT_GAMEPAD_LSTICKY,
        INPUT_GAMEPAD_LSTICKX,
        INPUT_GAMEPAD_GUIDE,

        KEY_UNKNOWN,
        COUNT
    };
}