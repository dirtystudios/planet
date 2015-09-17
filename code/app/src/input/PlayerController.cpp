#include "stdafx.h"
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

    void PlayerController::LookMode(float value) {
        lookMode = value > 0;
    }

    void PlayerController::MoveForward(float value) {
        if (value > 0) {
            moveInput.z += value * walkSpeed;
        }
    }

    void PlayerController::MoveBackward(float value) {
        if (value < 0) {
            moveInput.z += value * walkSpeed;
        }
    }

    void PlayerController::MoveLeft(float value) {
        if (value < 0) {
            moveInput.x += value * walkSpeed;
        }
    }

    void PlayerController::MoveRight(float value) {
        if (value > 0) {
            moveInput.x += value * walkSpeed;
        }
    }

    void PlayerController::LookUp(float value) {
        if (value > 0) {
            lookInput.y += value * lookSpeed;
        }
    }

    void PlayerController::LookDown(float value) {
        if (value < 0) {
            lookInput.y += value * lookSpeed;
        }
    }

    void PlayerController::LookRight(float value) {
        if (value > 0) {
            lookInput.x += value * lookSpeed;
        }
    }

    void PlayerController::LookLeft(float value) {
        if (value < 0) {
            lookInput.x += value * lookSpeed;
        }
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
