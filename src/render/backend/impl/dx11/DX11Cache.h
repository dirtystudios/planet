#pragma once

#include "ResourceTypes.h"
#include <unordered_map>

namespace gfx {
    struct DX11Cache {
        // these map to handles
        std::unordered_map<size_t, VertexLayoutId> ilCache;
        std::unordered_map<size_t, PipelineStateId> pipelineCache;
        // these are just hashes as only renderdevice uses them
        std::unordered_map<size_t, BlendStateDX11> bsCache;
        std::unordered_map<size_t, RasterStateDX11> rsCache;
        std::unordered_map<size_t, DepthStateDX11> dsCache;
    };
}