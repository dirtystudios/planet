
#include "PlayerController.h"


namespace controllers {
    PlayerController::PlayerController(Camera* camera,  input::InputContext *inputContext) {
        lookMode = false;
        m_inputContext = inputContext;
        cam = camera;
        moveInput = { 0.0f, 0.0f, 0.0f };
        walkSpeed = 300.0f;
        lookSpeed = 1.f;
        m_inputContext->BindContext<input::ContextBindingType::Axis>("MoveForward", BIND_MEM_CB(&PlayerController::MoveForward, this));
        m_inputContext->BindContext<input::ContextBindingType::Axis>("MoveBackward", BIND_MEM_CB(&PlayerController::MoveBackward, this));
        m_inputContext->BindContext<input::ContextBindingType::Axis>("MoveLeft", BIND_MEM_CB(&PlayerController::MoveLeft, this));
        m_inputContext->BindContext<input::ContextBindingType::Axis>("MoveRight", BIND_MEM_CB(&PlayerController::MoveRight, this));
        m_inputContext->BindContext<input::ContextBindingType::Axis>("LookUp", BIND_MEM_CB(&PlayerController::LookUp, this));
        m_inputContext->BindContext<input::ContextBindingType::Axis>("LookDown", BIND_MEM_CB(&PlayerController::LookDown, this));
        m_inputContext->BindContext<input::ContextBindingType::Axis>("LookRight", BIND_MEM_CB(&PlayerController::LookRight, this));
        m_inputContext->BindContext<input::ContextBindingType::Axis>("LookLeft", BIND_MEM_CB(&PlayerController::LookLeft, this));

        m_inputContext->BindContext<input::ContextBindingType::Action>("LookMode", BIND_MEM_CB(&PlayerController::LookMode, this));
    }

    bool PlayerController::LookMode(float value) {
        lookMode = value > 0;
        return true;
    }

    bool PlayerController::MoveForward(float value) {
        if (value > 0) {
            moveInput.z += value * walkSpeed;
        }
        return true;
    }

    bool PlayerController::MoveBackward(float value) {
        if (value < 0) {
            moveInput.z += value * walkSpeed;
        }
        return true;
    }

    bool PlayerController::MoveLeft(float value) {
        if (value < 0) {
            moveInput.x += value * walkSpeed;
        }
        return true;
    }

    bool PlayerController::MoveRight(float value) {
        if (value > 0) {
            moveInput.x += value * walkSpeed;
        }
        return true;
    }

    bool PlayerController::LookUp(float value) {
        if (value > 0) {
            lookInput.y += value * lookSpeed;
        }
        return lookMode;
    }

    bool PlayerController::LookDown(float value) {
        if (value < 0) {
            lookInput.y += value * lookSpeed;
        }
        return lookMode;
    }

    bool PlayerController::LookRight(float value) {
        if (value > 0) {
            lookInput.x += value * lookSpeed;
        }
        return lookMode;
    }

    bool PlayerController::LookLeft(float value) {
        if (value < 0) {
            lookInput.x += value * lookSpeed;
        }
        return lookMode;
    }

    void PlayerController::DoUpdate(float dt) {
        moveInput.x *= dt;
        moveInput.y *= dt;
        moveInput.z *= dt;
        cam->Translate(moveInput);

        if (lookMode) {
            cam->Pitch((lookInput.y * dt));
            cam->Yaw((lookInput.x * dt));
        }

        lookInput = { 0.0f, 0.0f };
        moveInput = { 0.0f, 0.0f, 0.0f };
    }
}
