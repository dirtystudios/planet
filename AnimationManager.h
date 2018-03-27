#pragma once

#include "MeshRenderObj.h"
#include "AnimationData.h"
#include <vector>
#include <unordered_map>

class MeshRenderer;
class SimObj;

class AnimationManager {
private:
    struct ManagedAnimation {
        std::unique_ptr<MeshRenderObj> meshRenderObj;
        AnimationData* animData;
        MeshPtr mesh;
        std::unordered_map<std::string, size_t> boneMapping;
    };

    MeshRenderer* m_meshRenderer;
    std::vector<ManagedAnimation> m_managedAnimations;
    float m_runningTime{ 0.f };

public:
    AnimationManager(MeshRenderer* meshRenderer)
        : m_meshRenderer(meshRenderer) {

    }

    void AddAnimationObj(SimObj* animObj);
    void DoUpdate(float ms);

private:
    glm::vec3 CalcInterpolatedScaling(float animTime, const AnimationData::AnimationNode& animNode);
    glm::quat CalcInterpolatedRotation(float animTime, const AnimationData::AnimationNode& animNode);
    glm::vec3 CalcInterpolatedTrans(float animTime, const AnimationData::AnimationNode& animNode);

    template<typename T>
    size_t GetAnimKeyframeItemIdx(const std::vector<T>& items, float time) {
        for (size_t i = 0; i < items.size() - 1; ++i) {
            if (time < items[i + 1].time)
                return i;
        }
        return 0;
    }
};