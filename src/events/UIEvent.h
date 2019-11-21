#pragma once

#include <string>
#include <vector>

struct UIEvent {
    std::string name;
    std::vector<std::string> args;
};