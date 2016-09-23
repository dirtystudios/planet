#pragma once
#include <glm/glm.hpp>
#include <functional>
#include "IndexedMeshData.h"

namespace meshGen {

using VertexDelegate = std::function<void(float, float, float, float, float)>;

IndexedMeshData CreateIcoSphere(uint32_t recursionLevel);
void GenerateGrid(float centerX, float centerY, float centerZ, float sizeX, float sizeY, uint32_t resolutionX, uint32_t resolutionY, VertexDelegate delegate);
void GenerateGrid(const glm::vec3& center, const glm::vec2& size, const glm::uvec2& resolution, VertexDelegate delegate);
void GenerateCube(VertexDelegate delegate, float originX, float originY, float originZ, float scale = 1.f, bool flip = true);
}
