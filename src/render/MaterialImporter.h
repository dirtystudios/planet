#pragma once
#include <vector>
#include <string>
#include "MaterialData.h"
#include <queue>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>

#include "Log.h"

namespace materialImport {
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
				aiString texturePath;
				material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath);
				matData->diffuseMap = std::string(texturePath.C_Str());
				LOG_D("aiTextureType_DIFFUSE:%s", matData->diffuseMap.c_str());

			}

			if (material->GetTextureCount(aiTextureType_SPECULAR)) {
				aiString texturePath;
				material->GetTexture(aiTextureType_SPECULAR, 0, &texturePath);
				matData->specularMap = std::string(texturePath.C_Str());
				LOG_D("aiTextureType_SPECULAR:%s", matData->specularMap.c_str());
			}

			aiColor3D color;
			material->Get(AI_MATKEY_SHADING_MODEL, matData->shadingModel);
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
