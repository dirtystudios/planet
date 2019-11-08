#pragma once

#include "InputManager.h"
#include <string>
#include <vector>

struct UIEvent {
    std::string name;
    std::vector<std::string> args;
};