#pragma once

#include <string>
#include <vector>

struct PacketEvent {
    std::string name;
    std::vector<std::string> args;
};