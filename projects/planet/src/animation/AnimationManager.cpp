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

void AnimationManager::AddAnimationObj(SimObj* animObj) {
    SkinnedMesh* skinnedMesh = animObj->GetComponent<SkinnedMesh>(ComponentType::SkinnedMesh);
    AnimationComponent* anim = animObj->GetComponent<AnimationComponent>(ComponentType::Animation);
    dg_assert_nm(skinnedMesh != nullptr);
    dg_assert_nm(anim != nullptr);

    ManagedAnimation managedAnim;
    managedAnim.meshRenderObj = std::make_unique<MeshRenderObj>(skinnedMesh->mesh, skinnedMesh->mat);
    managedAnim.animData = &anim->animData;
    managedAnim.mesh = skinnedMesh->mesh;

    managedAnim.meshRenderObj->transform()->scale(skinnedMesh->scale);

    // This should help a bit with speed at the cost of more memory usage
    size_t idx = 0;
    for (const auto &bone : skinnedMesh->mesh->GetBones()) {
        managedAnim.boneMapping[bone.first] = idx;
        idx++;
    }

    m_meshRenderer->Register(managedAnim.meshRenderObj.get());
    m_managedAnimations.emplace_back(std::move(managedAnim));
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

void AnimationManager::DoUpdate(float ms) {
    m_runningTime += ms;
    for (auto& anim : m_managedAnimations) {
        const AnimationData* animData = anim.animData;

        // todo: theres probly a way to cache this instead, but maybe it wouldnt matter much
        std::queue<std::pair<size_t, glm::mat4>> nodeQueue;
        nodeQueue.push({ 0, glm::mat4() });

        const BoneOffsetData& bones = anim.mesh->GetBones();
        const MeshTreeData& tree = anim.mesh->GetTree();
        //const auto& parts = anim.mesh->GetParts();
        const auto& meshNodes = anim.mesh->GetNodes();

        float tps = animData->ticksPerSecond == 0.f ? 25.f : (float)animData->ticksPerSecond;
        float timeInTicks = tps * (m_runningTime / 1000);
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

            auto boneCheck = anim.boneMapping.find(meshNode.name);
            if (boneCheck != anim.boneMapping.end())
                finalBoneOffsets[boneCheck->second] = glm::transpose(anim.mesh->GetGimt() * transform * bones[boneCheck->second].second);

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

        anim.meshRenderObj->boneOffsets(finalBoneOffsets);
    }
}