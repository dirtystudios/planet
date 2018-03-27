#include "AnimationImporter.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

#include <queue>

#include "AnimationData.h"

#include "DGAssert.h"
#include "Log.h"

std::unordered_map<std::string, AnimationData> animationImport::LoadAnimationDataFromFile(const std::string& fpath) {
    std::unordered_map<std::string, AnimationData> animInfo;

    Assimp::Importer import;

    const aiScene* scene = import.ReadFile(fpath,
        aiProcess_JoinIdenticalVertices
        | aiProcess_Triangulate
        | aiProcess_FlipUVs
        | aiProcess_FixInfacingNormals
        | aiProcess_GenNormals
        | aiProcess_OptimizeGraph
        | aiProcess_OptimizeMeshes
    );

    if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        LOG_E("ASSIMP %s", import.GetErrorString());
        return{};
    }

    for (uint32_t i = 0; i < scene->mNumAnimations; ++i) {
        const aiAnimation* pAnimation = scene->mAnimations[i];
        const std::string animName = std::string(pAnimation->mName.C_Str());

        LOG_D("Processing Animation: %s", animName.c_str());
        dg_assert(pAnimation->mNumMeshChannels == 0, "Mesh Animation Channels > 0 not supported");
        
        AnimationData animData;
        animData.ticksPerSecond = pAnimation->mTicksPerSecond;
        animData.duration = pAnimation->mDuration;

        for (uint32_t c = 0; c < pAnimation->mNumChannels; ++c) {
            const aiNodeAnim* channel  = pAnimation->mChannels[c];
            AnimationData::AnimationNode animNode;

            animNode.translations.reserve(channel->mNumPositionKeys);
            for (uint32_t p = 0; p < channel->mNumPositionKeys; ++p) {
                const aiVector3D& vec = channel->mPositionKeys[p].mValue;
                const double time = channel->mPositionKeys[p].mTime;
                animNode.translations.emplace_back(time, glm::vec3{ vec.x, vec.y, vec.z});
            }

            animNode.rotations.reserve(channel->mNumRotationKeys);
            for (uint32_t r = 0; r < channel->mNumRotationKeys; ++r) {
                const aiQuaternion& quat = channel->mRotationKeys[r].mValue;
                const double time = channel->mRotationKeys[r].mTime;
                animNode.rotations.emplace_back(time, glm::toMat4(glm::quat{ quat.w, quat.x, quat.y, quat.z }));
            }

            animNode.scales.reserve(channel->mNumScalingKeys);
            for (uint32_t s = 0; s < channel->mNumScalingKeys; ++s) {
                const aiVector3D& vec = channel->mScalingKeys[s].mValue;
                const double time = channel->mScalingKeys[s].mTime;
                animNode.scales.emplace_back(time, glm::vec3{ vec.x,vec.y, vec.z });
            }

            animData.animationNodes.insert({ std::string(channel->mNodeName.C_Str()), std::move(animNode) });
        }
        animInfo.insert({ animName, std::move(animData) });
    }

    return animInfo;
}