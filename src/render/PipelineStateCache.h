#pragma once

#include <cassert>
#include "RenderDevice.h"
#include "PipelineStateDesc.h"

class PipelineStateCache {
private:
    gfx::RenderDevice* _device;

public:
    PipelineStateCache(gfx::RenderDevice* device) : _device(device) {}

    gfx::ShaderId Get(const gfx::PipelineStateDesc& psd) {
        // TODO: actually cache
        gfx::PipelineStateId psId = _device->CreatePipelineState(psd);
        assert(psId);
        return psId;
    }
};
