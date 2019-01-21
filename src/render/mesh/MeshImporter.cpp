#include "MeshImporter.h"

#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <vector>
#include <algorithm>
#include <queue>
#include <map>
#include "Log.h"

#include "MeshNode.h"
#include "MeshGeometryData.h"
#include "BoundingBox.h"

static glm::mat4 ConvertMat4(aiMatrix4x4& im) {
    return{
        im.a1, im.b1, im.c1, im.d1,
        im.a2, im.b2, im.c2, im.d2,
        im.a3, im.b3, im.c3, im.d3,
        im.a4, im.b4, im.c4, im.d4
    };
}

MeshGeometryData ProcessMesh(const aiMesh* mesh, std::vector<std::pair<std::string, glm::mat4>>& boneOffsets, std::vector<dm::BoundingBox>& bboxs) {
    MeshGeometryData geometryData;
	geometryData.isSkinned = true;

    if(mesh->HasPositions()) {
        geometryData.positions.reserve(mesh->mNumVertices);
        for(uint32_t i = 0; i < mesh->mNumVertices; i++) {
            geometryData.positions.push_back(glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z));
        }
    }
    
    if(mesh->HasNormals()) {
        geometryData.normals.reserve(mesh->mNumVertices);
        for(uint32_t i = 0; i < mesh->mNumVertices; i++) {
            geometryData.normals.push_back(glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z));
        }
    }
    
    if (mesh->mTextureCoords[0]) {
        geometryData.texcoords.reserve(mesh->mNumVertices);
        for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
            geometryData.texcoords.push_back(glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y));
        }
    }

    if (mesh->HasBones()) {
        geometryData.boneIds.resize(mesh->mNumVertices);
        geometryData.weights.resize(mesh->mNumVertices);
        
        std::fill(geometryData.boneIds.begin(), geometryData.boneIds.end(), glm::uvec4{ 0, 0, 0, 0 });
        std::fill(geometryData.weights.begin(), geometryData.weights.end(), glm::vec4{ 0.f, 0.f, 0.f, 0.f });

        for (uint32_t i = 0; i < mesh->mNumBones; ++i) {
            const std::string bName(mesh->mBones[i]->mName.data);
            auto check = std::find_if(boneOffsets.begin(), boneOffsets.end(),
                [&](const auto& element) { return element.first == bName; });

            uint32_t boneIndex = 0;
            if (check != boneOffsets.end())
                boneIndex = std::distance(boneOffsets.begin(), check);
            else {
                boneIndex = boneOffsets.size();
                boneOffsets.emplace_back(bName, ConvertMat4(mesh->mBones[i]->mOffsetMatrix));
                bboxs.emplace_back();
            }
            for (uint32_t j = 0; j < mesh->mBones[i]->mNumWeights; ++j) {
                uint32_t vertId = mesh->mBones[i]->mWeights[j].mVertexId;
                for (int c = 0; c < 4; ++c) {
                    if (geometryData.weights[vertId][c] == 0.f) {
                        geometryData.boneIds[vertId][c] = boneIndex;
                        geometryData.weights[vertId][c] = mesh->mBones[i]->mWeights[j].mWeight;

                        // todo: bounding box without bones
                        // calculate bounding box for meshs without bone

                        auto& bbox = bboxs[boneIndex];
                        auto& v = mesh->mVertices[vertId];
                        bbox.min.x = std::fmin(bbox.min.x, v.x);
                        bbox.min.y = std::fmin(bbox.min.y, v.y);
                        bbox.min.z = std::fmin(bbox.min.z, v.z);


                        bbox.max.x = std::fmax(bbox.max.x, v.x);
                        bbox.max.y = std::fmax(bbox.max.y, v.y);
                        bbox.max.z = std::fmax(bbox.max.z, v.z);

                        break;
                    }
                }
            }
        }
    }

    // Process indices
    for(uint32_t i = 0; i < mesh->mNumFaces; ++i) {
        const aiFace& face = mesh->mFaces[i];
        assert(face.mNumIndices == 3);
        for(uint32_t j = 0; j < face.mNumIndices; ++j) {
            geometryData.indices.push_back(face.mIndices[j]);
        }
    }
    return geometryData;
}

const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const std::string& NodeName) {
    for (uint32_t i = 0; i < pAnimation->mNumChannels; i++) {
        const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];

        if (std::string(pNodeAnim->mNodeName.data) == NodeName) {
            return pNodeAnim;
        }
    }

    return NULL;
}

meshImport::MeshData meshImport::LoadMeshDataFromFile(const std::string& fpath) {

    std::vector<MeshNode> meshNodes;
    std::vector<MeshGeometryData> meshGeomData;
    std::vector<std::pair<std::string, glm::mat4>> boneInfo;
    std::vector<dm::BoundingBox> boundingBoxs;
    std::map<uint32_t, std::vector<uint32_t>> tree;
    Assimp::Importer import;

    const aiScene* scene = import.ReadFile(fpath,
        aiProcess_JoinIdenticalVertices
        | aiProcess_Triangulate
        | aiProcess_FlipUVs
        | aiProcess_FixInfacingNormals
        | aiProcess_GenNormals
        | aiProcess_OptimizeGraph
        | aiProcess_OptimizeMeshes
        | aiProcess_LimitBoneWeights
        | aiProcess_GenUVCoords
        | aiProcess_FindInstances
    );

    if(!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        LOG_E("ASSIMP %s", import.GetErrorString());
        return{};
    }
    
    // Process meshes 
    meshGeomData.reserve(scene->mNumMeshes);
    for (uint32_t i = 0; i < scene->mNumMeshes; ++i) {
        const aiMesh* mesh = scene->mMeshes[i];
        LOG_D("Mesh:%s", mesh->mName.C_Str());
        meshGeomData.emplace_back(ProcessMesh(mesh, boneInfo, boundingBoxs));
    }

    std::queue<aiNode*> dfsQueue;
    dfsQueue.push(scene->mRootNode);

    const glm::mat4 gimt = glm::inverse(ConvertMat4(scene->mRootNode->mTransformation));

    while(!dfsQueue.empty()) {
        auto node = dfsQueue.front();
        const std::string nodeName = std::string(node->mName.C_Str());
        LOG_D("Processing node:%s", nodeName.c_str());
        dfsQueue.pop();	

		const size_t currentIdx = meshNodes.size();
        meshNodes.emplace_back();
        MeshNode* meshNode = &meshNodes.back();

		meshNode->name = nodeName;
		meshNode->localTransform = ConvertMat4(node->mTransformation);

        meshNode->meshParts.reserve(node->mNumMeshes);
		for (uint32_t i = 0; i < node->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			uint32_t matIdx = mesh->mMaterialIndex;
            uint32_t meshIdx = node->mMeshes[i];
            meshNode->meshParts.emplace_back(matIdx, meshIdx);
		}

        std::vector<uint32_t> children(node->mNumChildren);
        for(uint32_t i = 0; i < node->mNumChildren; i++) {
            dfsQueue.push(node->mChildren[i]);
            children[i] = (currentIdx + dfsQueue.size());
        }

        tree.insert({ currentIdx, std::move(children) });
    }

    return{ meshNodes, meshGeomData, boneInfo, gimt, tree, boundingBoxs };
}