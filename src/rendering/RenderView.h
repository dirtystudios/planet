#pragma once

#include "Camera.h"
#include "Viewport.h"

struct RenderView {
	RenderView(Camera* camera, Viewport* viewport) : camera(camera), viewport(viewport) {};

	Camera* camera { nullptr };
	Viewport* viewport { nullptr };
};

