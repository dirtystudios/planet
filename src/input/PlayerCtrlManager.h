#pragma once
#include "InputContext.h"
#include "ComponentManager.h"
#include "SimulationManager.h"
#include <glm/glm.hpp>

struct Camera;

class PlayerCtrlManager : public ComponentManager {
private:
    input::InputContext* m_inputContext;
    glm::vec3 moveInput;
    glm::vec2 m_mouseLookInput;
    glm::vec2 m_controllerLookInput;
    glm::vec2 cntrldInput;
    float walkSpeed;
    float lookSpeed;

    bool m_clickLookMode;
    Camera* cam;

public:

    PlayerCtrlManager(Camera* camera, input::InputContext *inputContext);

    Camera* GetCam() { return cam; }

    void UpdateViewport(const Viewport& vp) override {}

    void DoUpdate(std::map<ComponentType, const std::array<std::unique_ptr<Component>, MAX_SIM_OBJECTS>*>& components, float ms) override;

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

    bool PlyrCntrlUp(const input::InputContextCallbackArgs& args);
    bool PlyrCntrlDown(const input::InputContextCallbackArgs& args);
    bool PlyrCntrlLeft(const input::InputContextCallbackArgs& args);
    bool PlyrCntrlRight(const input::InputContextCallbackArgs& args);
private:
    void HandleCameraMovement(float ms);
};