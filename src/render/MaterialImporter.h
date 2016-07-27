#pragma once
#include <vector>
#include <string>
#include "MaterialData.h"
#include <queue>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Log.h"

namespace materialImport {
    MaterialData LoadMaterialDataFromFile(const std::string& fpath) {
        MaterialData matData;
        Assimp::Importer import;
        const aiScene* scene = import.ReadFile(fpath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);
        if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            LOG_E("ASSIMP %s", import.GetErrorString());
            return matData;
        }

        std::queue<aiNode*> dfsQueue;
        dfsQueue.push(scene->mRootNode);

        assert(dfsQueue.size() == 1);

        aiNode* node = dfsQueue.front();
        LOG_D("Processing node:%s", node->mName.C_Str());
        dfsQueue.pop();

        //for (uint32_t i = 0; i < scene->mNumMaterials; ++i) {
        
        assert(scene->mNumMaterials == 2);
        // second one is the one we want
        aiMaterial* material = scene->mMaterials[1];
        aiString name;
        material->Get(AI_MATKEY_NAME, name);
        LOG_D("material:%s", name.C_Str());
        
        int illumination = 0;
        material->Get(AI_MATKEY_SHADING_MODEL, illumination);
        matData.illumination = illumination;
        
        aiColor3D color(0.f, 0.f, 0.f);
        material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
        matData.Kd = glm::vec3(color.r, color.g, color.b);

        material->Get(AI_MATKEY_COLOR_AMBIENT, color);
        matData.Ka = glm::vec3(color.r, color.g, color.b);

        material->Get(AI_MATKEY_COLOR_SPECULAR, color);
        matData.Ks = glm::vec3(color.r, color.g, color.b);

        material->Get(AI_MATKEY_COLOR_EMISSIVE, color);
        matData.Ke = glm::vec3(color.r, color.g, color.b);

        assert(aiReturn_SUCCESS == material->Get(AI_MATKEY_SHININESS, matData.Ns));

        aiString path;
        material->GetTexture(aiTextureType_DIFFUSE, 0, &path);

        std::string imagePath = fpath.substr(0, fpath.find_last_of("/")) + "/" + path.C_Str();

        matData.diffuseTexture.reset(new Image);
        LoadImageFromFile(imagePath.c_str(), matData.diffuseTexture.get());


            //break;
        //}

        return matData;
    }
}