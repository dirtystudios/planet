#include "MeshImporter.h"

#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <vector>
#include <queue>
#include "Log.h"

#include "MeshPart.h"

MeshGeometryData ProcessMesh(aiMesh* mesh, const aiScene* scene) {
    MeshGeometryData geometryData;

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

std::vector<MeshPart> meshImport::LoadMeshDataFromFile(const std::string& fpath) {

    std::vector<MeshPart> meshParts;
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
    if(!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        LOG_E("ASSIMP %s", import.GetErrorString());
        return{};
    }
    
    std::queue<aiNode*> dfsQueue;
    dfsQueue.push(scene->mRootNode);
    
    while(!dfsQueue.empty()) {
        aiNode* node = dfsQueue.front();
        LOG_D("Processing node:%s", node->mName.C_Str());
        dfsQueue.pop();	
       
        for(uint32_t i = 0; i < node->mNumMeshes; i++) {
            meshParts.emplace_back();
            MeshPart* part = &meshParts.back();

            aiString materialName;
            
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            part->matIdx = mesh->mMaterialIndex;
            LOG_D("Mesh:%s", mesh->mName.C_Str());

            part->geometryData = ProcessMesh(mesh, scene);
        }

        for(uint32_t i = 0; i < node->mNumChildren; i++) {
            dfsQueue.push(node->mChildren[i]);        	
        }	
    }

    return meshParts;
}