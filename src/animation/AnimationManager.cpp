#include "AnimationManager.h"
#include "SkinnedMesh.h"
#include "AnimationComponent.h"
#include "AnimationData.h"
#include "SimObj.h"
#include "MeshRenderer.h"

#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

#include <algorithm>
#include <queue>

void AnimationManager::AddAnimationObj(uint64_t key, SkinnedMesh* skinnedMesh, AnimationComponent* anim, Spatial* spatial) {
    dg_assert_nm(skinnedMesh != nullptr);
    dg_assert_nm(anim != nullptr);
    dg_assert_nm(spatial != nullptr);

    AnimationPtr cache = m_animationCache->Get(anim->cacheKey);
    dg_assert(cache != nullptr, "animation not found in cache");

    ManagedAnimation managedAnim;
    managedAnim.meshRenderObj = std::make_unique<MeshRenderObj>(skinnedMesh->mesh, skinnedMesh->mat);
    managedAnim.animation = cache;
    managedAnim.mesh = skinnedMesh->mesh;
    managedAnim.cacheKey = anim->cacheKey;
    managedAnim.type = anim->animationType;
    managedAnim.runningTime = 0.f;
    managedAnim.dir = spatial->direction;
    managedAnim.pos = spatial->pos;

    managedAnim.meshRenderObj->transform()->scale(skinnedMesh->scale);

    // This should help a bit with speed at the cost of more memory usage
    size_t idx = 0;
    for (const auto &bone : skinnedMesh->mesh->GetBones()) {
        managedAnim.boneMapping[bone.first] = idx;
        idx++;
    }

    m_meshRenderer->Register(managedAnim.meshRenderObj.get());
    m_managedAnimations[key] = std::move(managedAnim);
}

void AnimationManager::UpdateAnimationObj(uint64_t key, SkinnedMesh* skinnedMesh, AnimationComponent* anim, Spatial* spatial) {
    dg_assert_nm(skinnedMesh != nullptr);
    dg_assert_nm(anim != nullptr);
    dg_assert_nm(spatial != nullptr);

    auto& managedAnim = m_managedAnimations[key];

    if (managedAnim.cacheKey != anim->cacheKey) {
        AddAnimationObj(key, skinnedMesh, anim, spatial);
        return;
    }

    managedAnim.dir = spatial->direction;
    managedAnim.pos = spatial->pos;

    managedAnim.meshRenderObj->transform()->translate(spatial->pos);
    if (glm::length(spatial->direction) > 0.0)
        managedAnim.meshRenderObj->transform()->lookAt(spatial->pos, spatial->pos + spatial->direction, glm::vec3(0, 1, 0));

    if (managedAnim.type != anim->animationType) {
        managedAnim.type = anim->animationType;
        managedAnim.runningTime = 0.f;
    }
}

glm::vec3 AnimationManager::CalcInterpolatedScaling(float animTime, const AnimationData::AnimationNode& animNode) {
    if (animNode.scales.size() == 1)
        return animNode.scales[0].scale;

    size_t scaleIdx = GetAnimKeyframeItemIdx(animNode.scales, animTime);
    size_t nextIdx = scaleIdx + 1;

    float deltaTime = (float)animNode.scales[nextIdx].time - (float)animNode.scales[scaleIdx].time;

    return glm::lerp(animNode.scales[scaleIdx].scale, animNode.scales[nextIdx].scale, (float)((animTime - animNode.scales[scaleIdx].time) / deltaTime));
}

glm::quat AnimationManager::CalcInterpolatedRotation(float animTime, const AnimationData::AnimationNode& animNode) {
    if (animNode.rotations.size() == 1)
        return animNode.rotations[0].rot;

    size_t idx = GetAnimKeyframeItemIdx(animNode.rotations, animTime);
    size_t nextIdx = idx + 1;

    float deltaTime = (float)animNode.rotations[nextIdx].time - (float)animNode.rotations[idx].time;

    return glm::normalize(glm::slerp(animNode.rotations[idx].rot, animNode.rotations[nextIdx].rot, (float)((animTime - animNode.rotations[idx].time) / deltaTime)));
}

glm::vec3 AnimationManager::CalcInterpolatedTrans(float animTime, const AnimationData::AnimationNode& animNode) {
    if (animNode.translations.size() == 1)
        return animNode.translations[0].scale;

    size_t idx = GetAnimKeyframeItemIdx(animNode.translations, animTime);
    size_t nextIdx = idx + 1;

    float deltaTime = (float)animNode.translations[nextIdx].time - (float)animNode.translations[idx].time;

    return glm::lerp(animNode.translations[idx].scale, animNode.translations[nextIdx].scale, (float)((animTime - animNode.translations[idx].time) / deltaTime));
}

void AnimationManager::DoUpdate(std::map<ComponentType, const std::array<std::unique_ptr<Component>, MAX_SIM_OBJECTS>*>& components, float ms) {
    assert(components[ComponentType::SkinnedMesh] != nullptr);
    assert(components[ComponentType::Animation] != nullptr);
    assert(components[ComponentType::Spatial] != nullptr);
    assert(components[ComponentType::BoundingBox] != nullptr);

    auto& meshs = *reinterpret_cast<const std::array<std::unique_ptr<SkinnedMesh>, MAX_SIM_OBJECTS>*>(components[ComponentType::SkinnedMesh]);
    auto& anims = *reinterpret_cast<const std::array<std::unique_ptr<AnimationComponent>, MAX_SIM_OBJECTS>*>(components[ComponentType::Animation]);
    auto& spatials = *reinterpret_cast<const std::array<std::unique_ptr<Spatial>, MAX_SIM_OBJECTS>*>(components[ComponentType::Spatial]);
    auto& bboxs = *reinterpret_cast<const std::array<std::unique_ptr<BoundingBoxComponent>, MAX_SIM_OBJECTS>*>(components[ComponentType::BoundingBox]);

    for (size_t i = 0; i < MAX_SIM_OBJECTS; ++i) {
        SkinnedMesh* mesh = meshs[i].get();
        AnimationComponent* anim = anims[i].get();
        auto spatial = spatials[i].get();
        auto bbox = bboxs[i].get();
        
        if (mesh != nullptr && anim != nullptr && spatial != nullptr) {
            auto it = m_managedAnimations.find(i);
            if (it == m_managedAnimations.end())
                AddAnimationObj(i, mesh, anim, spatial);
            else 
                UpdateAnimationObj(i, mesh, anim, spatial);

            if (bbox != nullptr)
                bbox->bboxs = mesh->mesh->GetBBoxs();
        }
        else {
            m_managedAnimations.erase(i);
        }
    }

    DoUpdate(ms);
}

void AnimationManager::DoUpdate(float ms) {
    for (auto& anim : m_managedAnimations) {
        anim.second.runningTime += ms;

        // todo: actually use type...somehow
        std::string animKey = "";
        switch (anim.second.type) {
        case AnimationType::IDLE: animKey = "IDLE"; break;
        case AnimationType::ATTACK: animKey = "ATTACK"; break;
        case AnimationType::ATTACK_IDLE: animKey = "ATTACK_IDLE"; break;
        case AnimationType::WALKING: animKey = "WALKING"; break;
        default:
            break;
        }

        const AnimationData* animData = nullptr;
        auto it = anim.second.animation->animData.find(animKey);
        if (it != anim.second.animation->animData.end())
            animData = &it->second;
        else
            animData = &anim.second.animation->animData.begin()->second;

        dg_assert_nm(animData != nullptr);

        // todo: theres probly a way to cache this instead, but maybe it wouldnt matter much
        std::queue<std::pair<size_t, glm::mat4>> nodeQueue;
        nodeQueue.push({ 0, glm::mat4() });

        const BoneOffsetData& bones = anim.second.mesh->GetBones();
        const MeshTreeData& tree = anim.second.mesh->GetTree();
        const auto& meshNodes = anim.second.mesh->GetNodes();

        float tps = animData->ticksPerSecond == 0.f ? 25.f : (float)animData->ticksPerSecond;
        float timeInTicks = tps * (anim.second.runningTime / 1000);
        float animTime = fmod(timeInTicks, (float)animData->duration);

        std::vector<glm::mat4> finalBoneOffsets(bones.size());

        while (!nodeQueue.empty()) {
            const auto &node = nodeQueue.front();
            const MeshNode &meshNode = meshNodes[node.first];

            glm::mat4 transform = meshNode.localTransform;

            auto animCheck = animData->animationNodes.find(meshNode.name);
            if (animCheck != animData->animationNodes.end()) {
                const auto& animNode = animCheck->second;

                glm::vec3 scaleVec = CalcInterpolatedScaling(animTime, animNode);
                const glm::mat4 scale = glm::scale(glm::vec3{ scaleVec.x, scaleVec.y, scaleVec.z });

                glm::quat rotVec = CalcInterpolatedRotation(animTime, animNode);
                const glm::mat4 rot = glm::toMat4(glm::quat{ rotVec.w, rotVec.x, rotVec.y, rotVec.z });

                auto transVec = CalcInterpolatedTrans(animTime, animNode);
                const glm::mat4 trans = glm::translate(glm::vec3{ transVec.x, transVec.y, transVec.z });

                transform = trans * rot * scale;
            }

            transform = node.second * transform;

            auto boneCheck = anim.second.boneMapping.find(meshNode.name);
            if (boneCheck != anim.second.boneMapping.end())
                finalBoneOffsets[boneCheck->second] = glm::transpose(anim.second.mesh->GetGimt() * transform * bones[boneCheck->second].second);

			nodeQueue.pop();
			auto it = tree.find(node.first);
            if (it != tree.end()) {
                const auto& childs = it->second;
                for (auto child : childs)
                    nodeQueue.push({ child, transform });
            }
            else
                LOG_E("Animation::DoUpdate. tree doesnt contain node as parent, its probly screwed up.");
        }

        anim.second.meshRenderObj->boneOffsets(finalBoneOffsets);
    }
}