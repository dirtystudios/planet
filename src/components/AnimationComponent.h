#pragma once

#include "Component.h"
#include "AnimationData.h"
#include "ComponentType.h"

// This clashes with render animation.h otherwise...should just namespace this
class AnimationComponent: public Component {
public:
    AnimationData animData;
    static constexpr ComponentType type() { return ComponentType::Animation; }
};