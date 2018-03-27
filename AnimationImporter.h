#pragma once 

#include <unordered_map>
#include <string>
#include "Animation.h"

namespace animationImport {
    // Map of animation names and data needed to display them
    std::unordered_map<std::string, AnimationData> LoadAnimationDataFromFile(const std::string& fpath);
}