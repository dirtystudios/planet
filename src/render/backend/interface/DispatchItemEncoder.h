#pragma once

#include <cassert>
#include <stdint.h>
#include <vector>
#include "Bytebuffer.h"
#include "DispatchCall.h"
#include "DispatchItem.h"
#include "RenderDevice.h"
#include "ResourceTypes.h"
#include "StateGroup.h"

namespace gfx {
    class DispatchItemEncoder {
    public:
        static const DispatchItem* Encode(RenderDevice* device, const DispatchCall& drawCall, const StateGroup* const* stateGroups, uint32_t count);
        static const DispatchItem* Encode(RenderDevice* device, const DispatchCall& drawCall, const std::vector<const StateGroup*>& stateGroups);
    };
}
