#pragma once

#include "RendererType.h"

class RenderObj {
private:
    RendererType _type;

public:
    RenderObj(RendererType type)
        : _type(type){};

    RendererType GetRendererType() const { return _type; }
};
