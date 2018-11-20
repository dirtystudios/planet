#pragma once

#include "Component.h"
#include "ComponentType.h"

class PlayerControlled : public Component {
public:
    static constexpr ComponentType type() { return ComponentType::PlayerControlled; }
};