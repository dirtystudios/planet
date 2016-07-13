#pragma once

#include "DrawItem.h"
#include "DrawCall.h"
#include <stdint.h>
#include <vector>
#include "ResourceTypes.h"
#include "Bytebuffer.h"
#include "StateGroup.h"
#include "RenderDevice.h"
#include <cassert>

namespace gfx {
class DrawItemEncoder {
public:
    static const DrawItem* Encode(RenderDevice* device, const DrawCall& drawCall, const StateGroup* const* stateGroups,
                                  uint32_t count);
};
}
