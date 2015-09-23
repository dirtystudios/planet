#pragma once
#include "Controller.h"
#include "Camera.h"
#include "InputContext.h"

namespace controllers {
    class PlayerController : public Controller {
    private:
        input::InputContext* m_inputContext;
        glm::vec3 moveInput;
        glm::vec2 lookInput;
        float walkSpeed;
        float lookSpeed;
        
        bool lookMode;
    public:
        Camera* cam;

        PlayerController(Camera* camera, input::InputContext *inputContext);

        bool LookMode(float value);

        bool MoveForward(float value);
        bool MoveBackward(float value);
        bool MoveLeft(float value);
        bool MoveRight(float value);

        bool LookUp(float value);
        bool LookDown(float value);
        bool LookRight(float value);
        bool LookLeft(float value);

        void DoUpdate(float dt);
    };
};