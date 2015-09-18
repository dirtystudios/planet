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

        void LookMode(float value);

        void MoveForward(float value);
        void MoveBackward(float value);
        void MoveLeft(float value);
        void MoveRight(float value);

        void LookUp(float value);
        void LookDown(float value);
        void LookRight(float value);
        void LookLeft(float value);

        void DoUpdate(float dt);
    };
};