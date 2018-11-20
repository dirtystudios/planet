#include "PlayerCtrlManager.h"
#include "Camera.h"

PlayerCtrlManager::PlayerCtrlManager(Camera* camera, input::InputContext* inputContext) {
    m_clickLookMode = false;
    m_inputContext = inputContext;
    cam = camera;
    moveInput = { 0.0f, 0.0f, 0.0f };
    m_mouseLookInput = { 0.f, 0.f };
    walkSpeed = 550.0f;
    lookSpeed = 1.f;
    m_inputContext->BindContext<input::ContextBindingType::Axis>("MoveForward", std::bind(&PlayerCtrlManager::MoveForward, this, std::placeholders::_1));
    m_inputContext->BindContext<input::ContextBindingType::Axis>("MoveBackward", std::bind(&PlayerCtrlManager::MoveBackward, this, std::placeholders::_1));
    m_inputContext->BindContext<input::ContextBindingType::Axis>("MoveLeft", std::bind(&PlayerCtrlManager::MoveLeft, this, std::placeholders::_1));
    m_inputContext->BindContext<input::ContextBindingType::Axis>("MoveRight", std::bind(&PlayerCtrlManager::MoveRight, this, std::placeholders::_1));
    m_inputContext->BindContext<input::ContextBindingType::Axis>("ShimmyUp", std::bind(&PlayerCtrlManager::ShimmyUp, this, std::placeholders::_1));
    m_inputContext->BindContext<input::ContextBindingType::Axis>("ShimmyDown", std::bind(&PlayerCtrlManager::ShimmyDown, this, std::placeholders::_1));

    m_inputContext->BindContext<input::ContextBindingType::Axis>("LookUp", std::bind(&PlayerCtrlManager::LookUp, this, std::placeholders::_1));
    m_inputContext->BindContext<input::ContextBindingType::Axis>("LookDown", std::bind(&PlayerCtrlManager::LookDown, this, std::placeholders::_1));
    m_inputContext->BindContext<input::ContextBindingType::Axis>("LookRight", std::bind(&PlayerCtrlManager::LookRight, this, std::placeholders::_1));
    m_inputContext->BindContext<input::ContextBindingType::Axis>("LookLeft", std::bind(&PlayerCtrlManager::LookLeft, this, std::placeholders::_1));
    m_inputContext->BindContext<input::ContextBindingType::Action>("LookMode", std::bind(&PlayerCtrlManager::LookMode, this, std::placeholders::_1));

    m_inputContext->BindContext<input::ContextBindingType::Axis>("PlayerCntldUp", std::bind(&PlayerCtrlManager::PlyrCntrlUp, this, std::placeholders::_1));
    m_inputContext->BindContext<input::ContextBindingType::Axis>("PlayerCntldDown", std::bind(&PlayerCtrlManager::PlyrCntrlDown, this, std::placeholders::_1));
    m_inputContext->BindContext<input::ContextBindingType::Axis>("PlayerCntldLeft", std::bind(&PlayerCtrlManager::PlyrCntrlLeft, this, std::placeholders::_1));
    m_inputContext->BindContext<input::ContextBindingType::Axis>("PlayerCntldRight", std::bind(&PlayerCtrlManager::PlyrCntrlRight, this, std::placeholders::_1));

}

bool PlayerCtrlManager::LookMode(const input::InputContextCallbackArgs& args) {
    m_clickLookMode = args.value > 0;
    return true;
}

bool PlayerCtrlManager::MoveForward(const input::InputContextCallbackArgs& args) {
    if (args.value > 0) {
        moveInput.z += args.value * walkSpeed;
    }
    return true;
}

bool PlayerCtrlManager::MoveBackward(const input::InputContextCallbackArgs& args) {
    if (args.value < 0) {
        moveInput.z += args.value * walkSpeed;
    }
    return true;
}

bool PlayerCtrlManager::MoveLeft(const input::InputContextCallbackArgs& args) {
    if (args.value < 0) {
        moveInput.x += args.value * walkSpeed;
    }
    return true;
}

bool PlayerCtrlManager::MoveRight(const input::InputContextCallbackArgs& args) {
    if (args.value > 0) {
        moveInput.x += args.value * walkSpeed;
    }
    return true;
}

bool PlayerCtrlManager::ShimmyUp(const input::InputContextCallbackArgs& args) {
    if (args.value > 0) {
        moveInput.y += args.value * walkSpeed;
    }
    return true;
}

bool PlayerCtrlManager::ShimmyDown(const input::InputContextCallbackArgs& args) {
    if (args.value < 0) {
        moveInput.y += args.value * walkSpeed;
    }
    return true;
}

bool PlayerCtrlManager::LookUp(const input::InputContextCallbackArgs& args) {
    if (args.value < 0) {
        m_mouseLookInput.y += args.value * lookSpeed;
        if (args.fromController)
            m_controllerLookInput.y += args.value * lookSpeed;
    }
    return m_clickLookMode;
}

bool PlayerCtrlManager::LookDown(const input::InputContextCallbackArgs& args) {
    if (args.value > 0) {
        m_mouseLookInput.y += args.value * lookSpeed;
        if (args.fromController)
            m_controllerLookInput.y += args.value * lookSpeed;
    }
    return m_clickLookMode;
}

bool PlayerCtrlManager::LookRight(const input::InputContextCallbackArgs& args) {
    if (args.value < 0) {
        m_mouseLookInput.x += args.value * lookSpeed;
        if (args.fromController)
            m_controllerLookInput.x += args.value * lookSpeed;
    }
    return m_clickLookMode;
}

bool PlayerCtrlManager::LookLeft(const input::InputContextCallbackArgs& args) {
    if (args.value > 0) {
        m_mouseLookInput.x += args.value * lookSpeed;
        if (args.fromController)
            m_controllerLookInput.x += args.value * lookSpeed;
    }
    return m_clickLookMode;
}

void PlayerCtrlManager::HandleCameraMovement(float ms) {
    moveInput.x *= ms;
    moveInput.y *= ms;
    moveInput.z *= ms;
    cam->Translate(moveInput);

    cam->Pitch((m_controllerLookInput.y * ms));
    cam->Yaw((m_controllerLookInput.x * ms));

    if (m_clickLookMode) {
        cam->Pitch((m_mouseLookInput.y * ms));
        cam->Yaw((m_mouseLookInput.x * ms));
    }

    m_mouseLookInput = { 0.0f, 0.0f };
    m_controllerLookInput = { 0.f, 0.f };
    moveInput = { 0.0f, 0.0f, 0.0f };
}

bool PlayerCtrlManager::PlyrCntrlUp(const input::InputContextCallbackArgs& args) {
    if (args.value > 0) {
        cntrldInput.y += args.value;
    }
    return true;
}

bool PlayerCtrlManager::PlyrCntrlDown(const input::InputContextCallbackArgs& args) {
    if (args.value < 0) {
        cntrldInput.y += args.value;
    }
    return true;
}

bool PlayerCtrlManager::PlyrCntrlLeft(const input::InputContextCallbackArgs& args) {
    if (args.value > 0) {
        cntrldInput.x += args.value;
    }
    return true;
}

bool PlayerCtrlManager::PlyrCntrlRight(const input::InputContextCallbackArgs& args) {
    if (args.value < 0) {
        cntrldInput.x += args.value;
    }
    return true;
}

void PlayerCtrlManager::DoUpdate(std::map<ComponentType, const std::array<std::unique_ptr<Component>, MAX_SIM_OBJECTS>*>& components, float ms) {

    assert(components[ComponentType::PlayerControlled] != nullptr);
    assert(components[ComponentType::Spatial] != nullptr);
    assert(components[ComponentType::Animation] != nullptr);

    auto& pcs = *reinterpret_cast<const std::array<std::unique_ptr<PlayerControlled>, MAX_SIM_OBJECTS>*>(components[ComponentType::PlayerControlled]);
    auto& spatials = *reinterpret_cast<const std::array<std::unique_ptr<Spatial>, MAX_SIM_OBJECTS>*>(components[ComponentType::Spatial]);
    auto& anims = *reinterpret_cast<const std::array<std::unique_ptr<AnimationComponent>, MAX_SIM_OBJECTS>*>(components[ComponentType::Animation]);

    HandleCameraMovement(ms);

    for (size_t i = 0; i < MAX_SIM_OBJECTS; ++i) {
        PlayerControlled* pc = pcs[i].get();
        Spatial* spatial = spatials[i].get();
        AnimationComponent* anim = anims[i].get();

        if (pc != nullptr && spatial != nullptr) {
            spatial->direction = glm::dvec3(cntrldInput.x * -1.0, 0.0, cntrldInput.y);

            if (anim != nullptr) {
                anim->animationType = glm::length(cntrldInput) > 0.1 ? AnimationType::WALKING : AnimationType::IDLE;
            }
        }
    }

    cntrldInput = { 0.f, 0.f };
}