#pragma once
#include <functional>
#include <glm/glm.hpp>
#include "MeshGeometryData.h"

namespace dgen {

using VertexDelegate = std::function<void(float, float, float, float, float)>;

void GenerateIcoSphere(uint32_t recursionLevel, MeshGeometryData* geometry);
void GenerateGrid(float centerX, float centerY, float centerZ, float sizeX, float sizeY, uint32_t resolutionX, uint32_t resolutionY, MeshGeometryData* geometryData);
void GenerateGrid(const glm::vec3& center, const glm::vec2& size, const glm::uvec2& resolution, MeshGeometryData* geometryData);
void GenerateCube(VertexDelegate delegate, float originX, float originY, float originZ, float scale = 1.f, bool flip = true);
}
