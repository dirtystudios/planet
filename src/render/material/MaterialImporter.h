#pragma once
#include <vector>
#include <string>
#include "MaterialData.h"
#include <queue>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>
#include <unordered_map>

#include "StringUtil.h"
#include "Image.h"
#include "Log.h"
#include "File.h"

namespace materialImport {
    const static std::unordered_map<aiShadingMode, ShadingModel> AssimpToDirty = {
        { aiShadingMode_NoShading, ShadingModel::None },
        { aiShadingMode_Flat, ShadingModel::Flat },
        { aiShadingMode_Gouraud, ShadingModel::Gouraud },
        { aiShadingMode_CookTorrance, ShadingModel::CookTorrance },
        { aiShadingMode_Fresnel, ShadingModel::Fresnel },
        { aiShadingMode_Minnaert, ShadingModel::Minnaert },
        { aiShadingMode_OrenNayar, ShadingModel::OrenNayar },
        { aiShadingMode_Toon, ShadingModel::Toon },
        { aiShadingMode_Phong, ShadingModel::Phong },
        { aiShadingMode_Blinn, ShadingModel::Blinn },
    };


    std::vector<MaterialData> LoadMaterialDataFromFile(const std::string& fpath) {
        Assimp::Importer import;
        const aiScene* scene = import.ReadFile(fpath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);
        if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            LOG_E("ASSIMP %s", import.GetErrorString());
            return{};
        }

        std::vector<MaterialData> rtnData;

        rtnData.reserve(scene->mNumMaterials);
        for (uint32_t i = 0; i < scene->mNumMaterials; ++i) {
            rtnData.emplace_back();
            MaterialData* matData = &rtnData.back();

            aiMaterial* material = scene->mMaterials[i];

            aiString materialName;
            material->Get(AI_MATKEY_NAME, materialName);
            LOG_D("Material:%s", materialName.C_Str());
            matData->name = std::string(materialName.C_Str());

            if (material->GetTextureCount(aiTextureType_DIFFUSE)) {
                assert(material->GetTextureCount(aiTextureType_DIFFUSE) == 1);
                aiString texturePath;

                material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath);
                matData->diffuseMap = fs::SanitizeFilePath(std::string(texturePath.C_Str()));
                LOG_D("aiTextureType_DIFFUSE:%s", matData->diffuseMap.c_str());

                std::string imgPath = fs::DirName(fpath) + "/" + matData->diffuseMap.c_str();
                
                dimg::LoadImageFromFile(imgPath.c_str(), &matData->diffuseData);

            }

            if (material->GetTextureCount(aiTextureType_SPECULAR)) {
                // im not dealing with this for now
                assert(false);
                assert(material->GetTextureCount(aiTextureType_SPECULAR) == 1);
                aiString texturePath;
                material->GetTexture(aiTextureType_SPECULAR, 0, &texturePath);
                matData->specularMap = fs::SanitizeFilePath(std::string(texturePath.C_Str()));
                LOG_D("aiTextureType_SPECULAR:%s", matData->specularMap.c_str());
            }

            aiColor3D color;
            aiShadingMode shadingMode;
            material->Get(AI_MATKEY_SHADING_MODEL, shadingMode);
            auto it = AssimpToDirty.find(shadingMode);
            if (it == AssimpToDirty.end())
                matData->shadingModel = ShadingModel::Blinn;
            else 
                matData->shadingModel = it->second;

            material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
            matData->kd = glm::vec3(color.r, color.g, color.b);
            material->Get(AI_MATKEY_COLOR_AMBIENT, color);
            matData->ka = glm::vec3(color.r, color.g, color.b);
            material->Get(AI_MATKEY_COLOR_SPECULAR, color);
            matData->ks = glm::vec3(color.r, color.g, color.b);
            material->Get(AI_MATKEY_COLOR_EMISSIVE, color);
            matData->ke = glm::vec3(color.r, color.g, color.b);
            material->Get(AI_MATKEY_SHININESS_STRENGTH, matData->ns);
        }

        return rtnData;
    }
    }
