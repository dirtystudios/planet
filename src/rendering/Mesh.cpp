#include "Mesh.h"
#include <iostream>
// struct Texture {
//     uint32_t id;
//     std::string type;
//     aiString path; // We store the path of the texture to compare with    
// };

// vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName) {
// 	vector<Texture> textures;
// 	for(uint32_t i = 0; i < mat->GetTextureCount(type); i++) {
// 		aiString str;
// 		mat->GetTexture(type, i, &str);
// 		GLboolean skip = false;
// 		for(uint32_t j = 0; j < textures_loaded.size(); j++) {
// 				if(textures_loaded[j].path == str) {
//    				textures.push_back(textures_loaded[j]);
//    				skip = true;
//    				break;
// 			}
// 		 }
// 		if(!skip) { // If texture hasn’t been loaded already, load it
// 			Texture texture;
// 			// atexture.id = TextureFromFile(str.c_str(), this->directory);
// 			texture.type = typeName;
// 			texture.path = str;
// 			textures.push_back(texture);
// 			// this->textures_loaded.push_back(texture); // Add to loaded
// 			return textures;
// 		}
// 	}
// 	return textures;
// }




IndexedMeshData ProcessMesh(aiMesh* mesh, const aiScene* scene) {
	IndexedMeshData meshData;	

	std::vector<glm::vec3>* positions 	= &meshData.positions;	
	std::vector<glm::vec3>* normals 	= &meshData.normals;
	// std::vector<glm::vec2>* texcoords 	= &meshData.texcoords;
	std::vector<uint32_t>* indices 		= &meshData.indices;

	if(mesh->HasPositions()) {		
		for(uint32_t i = 0; i < mesh->mNumVertices; i++) { 		
			positions->push_back(glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z));			
		}
	}
	
	if(mesh->HasNormals()) {		
		for(uint32_t i = 0; i < mesh->mNumVertices; i++) { 						
			normals->push_back(glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z));
		}		
	}
	
	
	// if(mesh->mTextureCoords[0]) {
	// 	texcoords->reserve(mesh->mNumVertices);
	// 	for(uint32_t i = 0; i < mesh->mNumVertices; i++) { 		
	// 		glm::vec2 tex(mesh->mTextureCoords[i]->x, mesh->mTextureCoords[i]->y);		
	// 		texcoords->push_back(tex);		    
	// 	}
	// }
		

	// Process indices
	for(uint32_t i = 0; i < mesh->mNumFaces; ++i) {
		const aiFace& face = mesh->mFaces[i];
		for(uint32_t j = 0; j < face.mNumIndices; ++j) {
			indices->push_back(face.mIndices[j]);
		}
	}

	// // Process material
	// if(mesh->mMaterialIndex >= 0) {
	// 	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	// 	vector<Texture> diffuseMaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
	// 	textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
	// 	vector<Texture> specularMaps = this->loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
	// 	textures.insert(textures.end(), specularMaps.begin(), specularMaps.end()); 
	// }

    return meshData;
}

 

std::vector<IndexedMeshData> LoadMeshDataFromFile(const std::string& fpath) {	
	std::vector<IndexedMeshData> meshes;

	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(fpath, aiProcess_Triangulate | aiProcess_FlipUVs |  aiProcess_GenNormals);
	if(!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
	   std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
	   return meshes;
	}
	
	std::queue<aiNode*> dfsQueue;
	dfsQueue.push(scene->mRootNode);

	while(!dfsQueue.empty()) {
		aiNode* node = dfsQueue.front();
		printf("Processing node:%s\n", node->mName.C_Str());
		dfsQueue.pop();	

		for(uint32_t i = 0; i < node->mNumMeshes; i++) {
     	   	aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
     	   	IndexedMeshData data = ProcessMesh(mesh, scene);
     	   	printf("meshdata:%d\n", data.positions.size());
			meshes.push_back(data);
    	}

    	for(uint32_t i = 0; i < node->mNumChildren; i++) {
    		dfsQueue.push(node->mChildren[i]);        	
    	}	
	}	

	return meshes;
}