#pragma once

#include "Component.h"
#include "AnimationData.h"

// This clashes with render animation.h otherwise...should just namespace this
class AnimationComponent: public Component {
public:
    AnimationData animData;
};