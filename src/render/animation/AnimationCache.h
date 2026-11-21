#pragma once

#include "Animation.h"
#include "AnimationImporter.h"
#include "File.h"
#include "RenderCache.h"

struct AnimationCachePolicy {
    using CacheItemType = AnimationPtr;
    using FileDataType = std::unordered_map<std::string, AnimationData>;

    AnimationCachePolicy(gfx::RenderDevice* device) {};

    FileDataType LoadDataFromFile(const fs::path& fpath) {
        return animationImport::LoadAnimationDataFromFile(fpath);
    }

    CacheItemType ConstructCacheItem(FileDataType&& data) {
        return  std::make_shared<Animation>(std::move(data));
    }
};

using AnimationCache = RenderCache<AnimationCachePolicy>;