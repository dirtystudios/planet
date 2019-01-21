#pragma once

#include "MeshRenderObj.h"
#include "AnimationData.h"
#include "ComponentManager.h"
#include "SimulationManager.h"
#include "Animation.h"
#include "AnimationCache.h"
#include "EventManager.h"
#include "BoundingBox.h"
#include <vector>
#include <unordered_map>

class MeshRenderer;
class SimObj;

class AnimationManager : public ComponentManager {
private:
    struct ManagedAnimation {
        std::unique_ptr<MeshRenderObj> meshRenderObj;
        AnimationPtr animation;
        MeshPtr mesh;
        float runningTime;
        std::unordered_map<std::string, size_t> boneMapping;
        std::string cacheKey;
        AnimationType type;
        glm::dvec3 pos;
        glm::dvec3 dir;
        std::vector<dm::BoundingBox>* bboxs{ nullptr };
    };

    MeshRenderer* m_meshRenderer;
    AnimationCache* m_animationCache;
    std::map<uint64_t, ManagedAnimation> m_managedAnimations;

public:
    AnimationManager(EventManager* em, MeshRenderer* meshRenderer, AnimationCache* animationCache)
        : m_meshRenderer(meshRenderer)
        , m_animationCache(animationCache)
    {}

    void UpdateViewport(const Viewport& vp) override {}
    void DoUpdate(std::map<ComponentType, const std::array<std::unique_ptr<Component>, MAX_SIM_OBJECTS>*>& components, float ms) override;

private:
    void AddAnimationObj(uint64_t key, SkinnedMesh* skinnedMesh, AnimationComponent* anim, Spatial* spatial);
    void UpdateAnimationObj(uint64_t key, SkinnedMesh* skinnedMesh, AnimationComponent* anim, Spatial* spatial);
    void DoUpdate(float ms);

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