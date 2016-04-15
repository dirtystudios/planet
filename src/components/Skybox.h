#pragma once

#include "Component.h"

class Skybox : public Component {
public:
	enum class CubeFace : uint8_t {
		Left = 0, 
		Right,
		Up,
		Down,
		Front, 
		Back,
		Count
	};

	// TODO: Shoulnt paths. Need to abstract to 'Images'.
	std::string imagePaths[static_cast<size_t>(CubeFace::Count)];
	RenderObj* renderObj { nullptr };
};