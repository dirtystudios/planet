#pragma once

#include "Component.h"
#include "RenderObj.h"
#include <glm/glm.hpp>

class ChunkedTerrain : public Component {
public:
	float size;
	std::function<double(double, double, double)> heightmapGenerator;
	glm::mat4 translation;
	glm::mat4 rotation;
	RenderObj* renderObj;
};