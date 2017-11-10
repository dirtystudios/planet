#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "AnimationData.h"

struct Animation {
    Animation(std::unordered_map<std::string, AnimationData>&& datas) {
        animData = std::move(datas);
    }
    std::unordered_map<std::string, AnimationData> animData;
};

using AnimationPtr = std::shared_ptr<Animation>;