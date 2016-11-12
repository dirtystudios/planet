
#include "PlayerController.h"

namespace controllers {
    PlayerController::PlayerController(Camera* camera, input::InputContext* inputContext) {
        m_clickLookMode  = false;
        m_inputContext   = inputContext;
        cam              = camera;
        moveInput        = {0.0f, 0.0f, 0.0f};
        m_mouseLookInput = {0.f, 0.f};
        walkSpeed        = 550.0f;
        lookSpeed = 1.f;
        m_inputContext->BindContext<input::ContextBindingType::Axis>("MoveForward", std::bind(&PlayerController::MoveForward, this, std::placeholders::_1));
        m_inputContext->BindContext<input::ContextBindingType::Axis>("MoveBackward", std::bind(&PlayerController::MoveBackward, this, std::placeholders::_1));
        m_inputContext->BindContext<input::ContextBindingType::Axis>("MoveLeft", std::bind(&PlayerController::MoveLeft, this, std::placeholders::_1));
        m_inputContext->BindContext<input::ContextBindingType::Axis>("MoveRight", std::bind(&PlayerController::MoveRight, this, std::placeholders::_1));
        m_inputContext->BindContext<input::ContextBindingType::Axis>("ShimmyUp", std::bind(&PlayerController::ShimmyUp, this, std::placeholders::_1));
        m_inputContext->BindContext<input::ContextBindingType::Axis>("ShimmyDown", std::bind(&PlayerController::ShimmyDown, this, std::placeholders::_1));

        m_inputContext->BindContext<input::ContextBindingType::Axis>("LookUp", std::bind(&PlayerController::LookUp, this, std::placeholders::_1));
        m_inputContext->BindContext<input::ContextBindingType::Axis>("LookDown", std::bind(&PlayerController::LookDown, this, std::placeholders::_1));
        m_inputContext->BindContext<input::ContextBindingType::Axis>("LookRight", std::bind(&PlayerController::LookRight, this, std::placeholders::_1));
        m_inputContext->BindContext<input::ContextBindingType::Axis>("LookLeft", std::bind(&PlayerController::LookLeft, this, std::placeholders::_1));
        m_inputContext->BindContext<input::ContextBindingType::Action>("LookMode", std::bind(&PlayerController::LookMode, this, std::placeholders::_1));
    }

    bool PlayerController::LookMode(const input::InputContextCallbackArgs& args) {
        m_clickLookMode = args.value > 0;
        return true;
    }

    bool PlayerController::MoveForward(const input::InputContextCallbackArgs& args) {
        if (args.value > 0) {
            moveInput.z += args.value * walkSpeed;
        }
        return true;
    }

    bool PlayerController::MoveBackward(const input::InputContextCallbackArgs& args) {
        if (args.value < 0) {
            moveInput.z += args.value * walkSpeed;
        }
        return true;
    }

    bool PlayerController::MoveLeft(const input::InputContextCallbackArgs& args) {
        if (args.value < 0) {
            moveInput.x += args.value * walkSpeed;
        }
        return true;
    }

    bool PlayerController::MoveRight(const input::InputContextCallbackArgs& args) {
        if (args.value > 0) {
            moveInput.x += args.value * walkSpeed;
        }
        return true;
    }

    bool PlayerController::ShimmyUp(const input::InputContextCallbackArgs& args) {
        if (args.value > 0) {
            moveInput.y += args.value * walkSpeed;
        }
        return true;
    }

    bool PlayerController::ShimmyDown(const input::InputContextCallbackArgs& args) {
        if (args.value < 0) {
            moveInput.y += args.value * walkSpeed;
        }
        return true;
    }

    bool PlayerController::LookUp(const input::InputContextCallbackArgs& args) {
        if (args.value < 0) {
            m_mouseLookInput.y += args.value * lookSpeed;
            if (args.fromController)
                m_controllerLookInput.y += args.value * lookSpeed;
        }
        return m_clickLookMode;
    }

    bool PlayerController::LookDown(const input::InputContextCallbackArgs& args) {
        if (args.value > 0) {
            m_mouseLookInput.y += args.value * lookSpeed;
            if (args.fromController)
                m_controllerLookInput.y += args.value * lookSpeed;
        }
        return m_clickLookMode;
    }

    bool PlayerController::LookRight(const input::InputContextCallbackArgs& args) {
        if (args.value < 0) {
            m_mouseLookInput.x += args.value * lookSpeed;
            if (args.fromController)
                m_controllerLookInput.x += args.value * lookSpeed;
        }
        return m_clickLookMode;
    }

    bool PlayerController::LookLeft(const input::InputContextCallbackArgs& args) {
        if (args.value > 0) {
            m_mouseLookInput.x += args.value * lookSpeed;
            if (args.fromController)
                m_controllerLookInput.x += args.value * lookSpeed;
        }
        return m_clickLookMode;
    }

    void PlayerController::DoUpdate(float dt) {
        moveInput.x *= dt;
        moveInput.y *= dt;
        moveInput.z *= dt;
        cam->Translate(moveInput);

        cam->Pitch((m_controllerLookInput.y * dt));
        cam->Yaw((m_controllerLookInput.x * dt));

        if (m_clickLookMode) {
            cam->Pitch((m_mouseLookInput.y * dt));
            cam->Yaw((m_mouseLookInput.x * dt));
        }

        m_mouseLookInput = { 0.0f, 0.0f };
        m_controllerLookInput = { 0.f, 0.f };
        moveInput = { 0.0f, 0.0f, 0.0f };
    }
}
