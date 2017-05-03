#include "InputUWP.h"

using namespace Windows::Foundation;
using namespace Windows::Gaming::Input;
using namespace Windows::UI::Core;

InputUWP::InputUWP(_In_ CoreWindow^ window) :
    m_activeGamepad(nullptr),
    m_gamepadsChanged(true)
{
    InitWindow(window);
}

void InputUWP::InitWindow(_In_ CoreWindow^ window)
{
    // Detect gamepad connection and disconnection events.
    Gamepad::GamepadAdded +=
        ref new EventHandler<Gamepad^>(this, &InputUWP::OnGamepadAdded);

    Gamepad::GamepadRemoved +=
        ref new EventHandler<Gamepad^>(this, &InputUWP::OnGamepadRemoved);
}


void InputUWP::Update(std::vector<float> &inputValues) {
    if (m_gamepadsChanged) {
        m_gamepadsChanged = false;
        unsigned int index = 0;

        if (Gamepad::Gamepads->Size == 0) {
            m_activeGamepad = nullptr;
        }
        // Check if the cached gamepad is still connected.
        else if (!Gamepad::Gamepads->IndexOf(m_activeGamepad, &index)) {
            // default to just the first single player gamepad
            m_activeGamepad = Gamepad::Gamepads->GetAt(0);
        }
    }

    if (m_activeGamepad == nullptr)
    {
        return;
    }

    GamepadReading reading = m_activeGamepad->GetCurrentReading();
    //todo, loop or some shit

    inputValues[(int)input::InputCode::INPUT_GAMEPAD_DOWN] = (reading.Buttons & GamepadButtons::DPadDown) == GamepadButtons::DPadDown ? 1.f : 0.f;
    inputValues[(int)input::InputCode::INPUT_GAMEPAD_UP] = (reading.Buttons & GamepadButtons::DPadUp) == GamepadButtons::DPadUp ? 1.f : 0.f;
    inputValues[(int)input::InputCode::INPUT_GAMEPAD_LEFT] = (reading.Buttons & GamepadButtons::DPadLeft) == GamepadButtons::DPadLeft ? 1.f : 0.f;
    inputValues[(int)input::InputCode::INPUT_GAMEPAD_RIGHT] = (reading.Buttons & GamepadButtons::DPadRight) == GamepadButtons::DPadRight ? 1.f : 0.f;

    inputValues[(int)input::InputCode::INPUT_GAMEPAD_A] = (reading.Buttons & GamepadButtons::A) == GamepadButtons::A ? 1.f : 0.f;
    inputValues[(int)input::InputCode::INPUT_GAMEPAD_B] = (reading.Buttons & GamepadButtons::B) == GamepadButtons::B ? 1.f : 0.f;
    inputValues[(int)input::InputCode::INPUT_GAMEPAD_X] = (reading.Buttons & GamepadButtons::X) == GamepadButtons::X ? 1.f : 0.f;
    inputValues[(int)input::InputCode::INPUT_GAMEPAD_Y] = (reading.Buttons & GamepadButtons::Y) == GamepadButtons::Y ? 1.f : 0.f;

    inputValues[(int)input::InputCode::INPUT_GAMEPAD_SELECT] = (reading.Buttons & GamepadButtons::View) == GamepadButtons::View ? 1.f : 0.f;
    inputValues[(int)input::InputCode::INPUT_GAMEPAD_START] = (reading.Buttons & GamepadButtons::Menu) == GamepadButtons::Menu ? 1.f : 0.f;

    inputValues[(int)input::InputCode::INPUT_GAMEPAD_RSHOULDER] = (reading.Buttons & GamepadButtons::RightShoulder) == GamepadButtons::RightShoulder ? 1.f : 0.f;
    inputValues[(int)input::InputCode::INPUT_GAMEPAD_LSHOULDER] = (reading.Buttons & GamepadButtons::LeftShoulder) == GamepadButtons::LeftShoulder ? 1.f : 0.f;

    // y axis is opposite then what SDL is
    inputValues[(int)input::InputCode::INPUT_GAMEPAD_LSTICKX] = static_cast<float>(reading.LeftThumbstickX);
    inputValues[(int)input::InputCode::INPUT_GAMEPAD_LSTICKY] = static_cast<float>(reading.LeftThumbstickY * -1.f);

    inputValues[(int)input::InputCode::INPUT_GAMEPAD_RSTICKX] = static_cast<float>(reading.RightThumbstickX);
    inputValues[(int)input::InputCode::INPUT_GAMEPAD_RSTICKY] = static_cast<float>(reading.RightThumbstickY * -1.f);
}

//----------------------------------------------------------------------

void InputUWP::OnGamepadAdded(_In_ Object^ sender, _In_ Gamepad^ gamepad) {
    // OnGamepadAdded and OnGamepadRemoved can be called from a worker thread. For simplicity,
    // defer updating the active gamepad until Update().
    m_gamepadsChanged = true;
}

//----------------------------------------------------------------------

void InputUWP::OnGamepadRemoved(_In_ Object^ sender, _In_ Gamepad^ gamepad) {
    // OnGamepadAdded and OnGamepadRemoved can be called from a worker thread. For simplicity,
    // defer updating the active gamepad until Update().
    m_gamepadsChanged = true;
}