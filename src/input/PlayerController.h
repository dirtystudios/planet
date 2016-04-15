#pragma once
#include "Controller.h"
#include "Camera.h"
#include "InputContext.h"

namespace controllers {
    class PlayerController : public Controller {
    private:
        input::InputContext* m_inputContext;
        glm::vec3 moveInput;
        glm::vec2 m_mouseLookInput;
        glm::vec2 m_controllerLookInput;
        float walkSpeed;
        float lookSpeed;
        
        bool m_clickLookMode;
    public:
        Camera* cam;

        PlayerController(Camera* camera, input::InputContext *inputContext);

        bool LookMode(const input::InputContextCallbackArgs& args);

        bool MoveForward(const input::InputContextCallbackArgs& args);
        bool MoveBackward(const input::InputContextCallbackArgs& args);
        bool MoveLeft(const input::InputContextCallbackArgs& args);
        bool MoveRight(const input::InputContextCallbackArgs& args);
        bool ShimmyUp(const input::InputContextCallbackArgs& args);
        bool ShimmyDown(const input::InputContextCallbackArgs& args);

        bool LookUp(const input::InputContextCallbackArgs& args);
        bool LookDown(const input::InputContextCallbackArgs& args);
        bool LookRight(const input::InputContextCallbackArgs& args);
        bool LookLeft(const input::InputContextCallbackArgs& args);

        void DoUpdate(float dt);
    };
};