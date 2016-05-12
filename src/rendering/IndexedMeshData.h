#pragma once

#include <glm/glm.hpp>
#include <vector>

struct IndexedMeshData {	
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texcoords;
	std::vector<uint32_t> indices;
};