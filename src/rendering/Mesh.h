#pragma once 

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <queue>
#include "ResourceTypes.h"




struct Mesh {
	graphics::BufferId vertexBuffer {0};
	graphics::BufferId indexBuffer {0};
	uint32_t indexCount {0};
	uint32_t indexOffset {0};
};


struct IndexedMeshData {	
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texcoords;
	std::vector<uint32_t> indices;
};


IndexedMeshData ProcessMesh(aiMesh* mesh, const aiScene* scene);
std::vector<IndexedMeshData> LoadMeshDataFromFile(const std::string& fpath);