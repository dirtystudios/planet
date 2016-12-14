#pragma once

#include <cassert>
#include <stdint.h>
#include <vector>
#include <vector>
#include "Bytebuffer.h"
#include "DrawCall.h"
#include "DrawItem.h"
#include "RenderDevice.h"
#include "ResourceTypes.h"
#include "StateGroup.h"

namespace gfx {
class DrawItemEncoder {
public:
    static const DrawItem* Encode(RenderDevice* device, const DrawCall& drawCall, const StateGroup* const* stateGroups, uint32_t count);
    static const DrawItem* Encode(RenderDevice* device, const DrawCall& drawCall, const std::vector<const StateGroup*>& stateGroups);
    static const DrawItem* Encode(RenderDevice* device, const DrawCall& drawCall, const StateGroup* stateGroup);
};
}
