#pragma once

#include "IdObj.h"
#include "RendererType.h"

class RenderObj : public IdObj {
private:
	RendererType _type;
public:
	RenderObj(RendererType type) : _type(type) {};

	RendererType GetRendererType() const {
		return _type;
	}
};