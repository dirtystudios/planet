#pragma once

#include "Component.h"
#include "RenderObj.h"

class ChunkedTerrain : public Component {
public:
	float size;
	std::function<double(double, double, double)> heightmapGenerator;

	RenderObj* renderObj;
};