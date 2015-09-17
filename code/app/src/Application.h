#ifndef __application_h__
#define __application_h__

#include "rendering/backend/RenderDevice.h"
#include "input/InputCodes.h"

namespace app {
    enum class KeyCode {
            KEY_A = 1,
            KEY_B = 2,
            KEY_C,
            KEY_D,
            KEY_E,
            KEY_F,
            KEY_G,
            KEY_H,
            KEY_I,
            KEY_J,
            KEY_K,
            KEY_L,
            KEY_M,
            KEY_N,
            KEY_O,
            KEY_P,
            KEY_Q,
            KEY_R,
            KEY_S,
            KEY_T,
            KEY_U,
            KEY_V,
            KEY_W,
            KEY_X,
            KEY_Y,
            KEY_Z,
            KEY_1,
            KEY_2,
            KEY_3,
            KEY_4,
            KEY_5,
            KEY_6,
            KEY_7,
            KEY_8,
            KEY_9,
            KEY_0,
            KEY_LEFT_SHIFT,
            KEY_UNKNOWN
    };

    struct KeyState {
        bool prev_pressed[256];
        bool pressed[256];

        bool IsPressed(app::KeyCode key) const {
            return pressed[(int)key];
        };

        bool OnPressed(app::KeyCode key) const {
            return pressed[(int)key] && !prev_pressed[(int)key];
        }

        bool OnReleased(app::KeyCode key) const {
            return !pressed[(int)key] && prev_pressed[(int)key];   
        }
    };

    struct CursorState {
        bool entered;
        double delta_x;
        double delta_y;
    };

    enum class ControllerKeyCode : uint32_t {
        KEY_Y = 0,
        KEY_X,
        KEY_A,
        KEY_B,
        KEY_HAMBURGER, // 'Start'
        KEY_CHEESE,    // 'Select'
        KEY_UP,
        KEY_DOWN,
        KEY_RIGHT,
        KEY_LEFT,
        KEY_RSTICK,
        KEY_LSTICK,
        KEY_RSHOULDER,
        KEY_LSHOULDER,
        KEY_GUIDE,
        COUNT,
    };

    struct ControllerKeyState {
        bool pressed[(uint32_t)ControllerKeyCode::COUNT];
        bool isPressed(app::ControllerKeyCode key) const {
            return pressed[(uint32_t)key];
        };
    };

    struct ControllerStick {
        double x;
        double y;
    };

    struct ControllerState {
        ControllerKeyState keyState;
        ControllerStick leftStick;
        ControllerStick rightStick;
        double rightTrigger;
        double leftTrigger;
    };

    struct AppState {    
        KeyState key_state;
        CursorState cursor_state;
        ControllerState controllerState;
    };

    class Application {
    public:
        graphics::RenderDevice* renderDevice;
        virtual void OnStart() = 0;
        virtual void OnFrame(const std::vector<float>& inputValues, float dt) = 0;
        virtual void OnShutdown() = 0;
    };
}

#endif