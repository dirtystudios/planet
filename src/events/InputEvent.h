#pragma once

#include "InputManager.h"
#include <string>

struct InputEvent {
    input::InputManager::ContextPriority context;
    input::ContextBindingType type;
    std::string name;
    input::InputContextCallbackArgs args;
};