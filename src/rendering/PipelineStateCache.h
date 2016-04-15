#pragma once

#include <cassert>
#include "RenderDevice.h"
#include "PipelineStateDesc.h"

class PipelineStateCache {
private:
    graphics::RenderDevice* _device;

public:
    PipelineStateCache(graphics::RenderDevice* device) : _device(device) {}

    graphics::ShaderId Get(const graphics::PipelineStateDesc& psd) {
        // TODO: actually cache
        graphics::PipelineStateId psId = _device->CreatePipelineState(psd);
        assert(psId);
        return psId;
    }
};