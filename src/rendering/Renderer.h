#pragma once

#include "SimObj.h"
#include "RenderObj.h"
#include "RenderView.h"

class Renderer {
public:
	virtual RenderObj* Register(SimObj* simObj) = 0;
	virtual void Unregister(RenderObj* renderObj) = 0;
	virtual void Submit(RenderView* renderView) = 0;
};