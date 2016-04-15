
#include "PlayerController.h"


namespace controllers {
    PlayerController::PlayerController(Camera* camera,  input::InputContext *inputContext) {
        m_clickLookMode = false;
        m_inputContext = inputContext;
        cam = camera;
        moveInput = { 0.0f, 0.0f, 0.0f };
        m_mouseLookInput = { 0.f, 0.f };
        walkSpeed = 300.0f;
        lookSpeed = 1.f;
        m_inputContext->BindContext<input::ContextBindingType::Axis>("MoveForward", BIND_MEM_CB(&PlayerController::MoveForward, this));
        m_inputContext->BindContext<input::ContextBindingType::Axis>("MoveBackward", BIND_MEM_CB(&PlayerController::MoveBackward, this));
        m_inputContext->BindContext<input::ContextBindingType::Axis>("MoveLeft", BIND_MEM_CB(&PlayerController::MoveLeft, this));
        m_inputContext->BindContext<input::ContextBindingType::Axis>("MoveRight", BIND_MEM_CB(&PlayerController::MoveRight, this));
        m_inputContext->BindContext<input::ContextBindingType::Axis>("ShimmyUp", BIND_MEM_CB(&PlayerController::ShimmyUp, this));
        m_inputContext->BindContext<input::ContextBindingType::Axis>("ShimmyDown", BIND_MEM_CB(&PlayerController::ShimmyDown, this));

        m_inputContext->BindContext<input::ContextBindingType::Axis>("LookUp", BIND_MEM_CB(&PlayerController::LookUp, this));
        m_inputContext->BindContext<input::ContextBindingType::Axis>("LookDown", BIND_MEM_CB(&PlayerController::LookDown, this));
        m_inputContext->BindContext<input::ContextBindingType::Axis>("LookRight", BIND_MEM_CB(&PlayerController::LookRight, this));
        m_inputContext->BindContext<input::ContextBindingType::Axis>("LookLeft", BIND_MEM_CB(&PlayerController::LookLeft, this));

        m_inputContext->BindContext<input::ContextBindingType::Action>("LookMode", BIND_MEM_CB(&PlayerController::LookMode, this));
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
