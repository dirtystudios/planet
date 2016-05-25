#pragma once

#include "DrawItem.h"
#include "Binding.h"

namespace graphics {
    class CommandBuffer {
    public:
        virtual void Clear(float r, float g, float b, float a) = 0;
        virtual void BindResource(const Binding& binding) = 0;
        virtual void DrawItem(const DrawItem* drawItem) = 0;
        virtual void Reset() = 0;
    };
}
