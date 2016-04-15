#pragma once
#include "input/InputCodes.h"

#include <atomic>
#include <vector>

// This file is based on the one found in uwp examples simple3dgameDX

// Analog control deadzone definitions. Tune these values to adjust the size of the deadzone.
// Thumbstick range in each dimension is defined as [-1, 1].
#define THUMBSTICK_DEADZONE 0.25f

// Trigger range is defined as [0, 1].
#define TRIGGER_DEADZONE 0.1f

ref class InputUWP {
private:
    // Game controller related members.
    Windows::Gaming::Input::Gamepad^    m_activeGamepad;
    std::atomic<bool>                   m_gamepadsChanged;
internal:
    InputUWP(_In_ Windows::UI::Core::CoreWindow^ window);

    void InitWindow(_In_ Windows::UI::Core::CoreWindow^ window);

    void Update(std::vector<float> &inputValues);

private:

    void OnGamepadAdded(
        _In_ Platform::Object^ sender,
        _In_ Windows::Gaming::Input::Gamepad^ gamepad
    );
    void OnGamepadRemoved(
        _In_ Platform::Object^ sender,
        _In_ Windows::Gaming::Input::Gamepad^ gamepad
    );
};