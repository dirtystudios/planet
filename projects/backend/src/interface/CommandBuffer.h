#pragma once

#include "DrawItem.h"

namespace gfx {
class CommandBuffer {
private:
    std::vector<const DrawItem*> _drawItems;

public:
    void DrawItem(const DrawItem* drawItem) { _drawItems.push_back(drawItem); }

    const std::vector<const struct DrawItem*>* GetDrawItems() const { return &_drawItems; }

    void Reset() { _drawItems.clear(); }
};
}
