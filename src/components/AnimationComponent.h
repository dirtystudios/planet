#pragma once

#include "Component.h"
#include "AnimationData.h"
#include "ComponentType.h"
#include "Animation.h"

// This clashes with render animation.h otherwise...should just namespace this

enum class AnimationType : uint32_t {
    IDLE = 0,
    ATTACK,
    ATTACK_IDLE,
    WALKING,
    COUNT
};

class AnimationComponent: public Component {
public:
    AnimationType animationType {AnimationType::IDLE};
    std::string cacheKey{""};
    static constexpr ComponentType type() { return ComponentType::Animation; }
};