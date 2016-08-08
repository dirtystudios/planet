#pragma once

#include <cassert>
#include "RenderDevice.h"
#include "VertexLayoutDesc.h"

class VertexLayoutCache {
private:
    gfx::RenderDevice* _device;

public:
    VertexLayoutCache(gfx::RenderDevice* device) : _device(device) {}

    gfx::VertexLayoutId Get(const gfx::VertexLayoutDesc& vld) {
        // TODO: actually cache
        gfx::VertexLayoutId vlId = _device->CreateVertexLayout(vld);
        assert(vlId);
        return vlId;
    }

    gfx::VertexLayoutId Pos3fNormal3fTex2f() {
        static gfx::VertexLayoutDesc layout{{{gfx::VertexAttributeType::Float3, gfx::VertexAttributeUsage::Position, gfx::VertexAttributeStorage::Float},
                                             {gfx::VertexAttributeType::Float3, gfx::VertexAttributeUsage::Normal, gfx::VertexAttributeStorage::Float},
                                             {gfx::VertexAttributeType::Float2, gfx::VertexAttributeUsage::Texcoord0, gfx::VertexAttributeStorage::Float}}};

        return Get(layout);
    }

    gfx::VertexLayoutId Pos3fNormal3f() {
        static gfx::VertexLayoutDesc layout{{{gfx::VertexAttributeType::Float3, gfx::VertexAttributeUsage::Position, gfx::VertexAttributeStorage::Float},
                                             {gfx::VertexAttributeType::Float3, gfx::VertexAttributeUsage::Normal, gfx::VertexAttributeStorage::Float}}};

        return Get(layout);
    }

    gfx::VertexLayoutId Pos3f() {
        static gfx::VertexLayoutDesc layout{{
            {gfx::VertexAttributeType::Float3, gfx::VertexAttributeUsage::Position, gfx::VertexAttributeStorage::Float},

        }};

        return Get(layout);
    }

    gfx::VertexLayoutId GetPos2fTex2f() {
        static gfx::VertexLayoutDesc layout{{{gfx::VertexAttributeType::Float2, gfx::VertexAttributeUsage::Position, gfx::VertexAttributeStorage::Float},
                                             {gfx::VertexAttributeType::Float2, gfx::VertexAttributeUsage::Texcoord0, gfx::VertexAttributeStorage::Float}

        }};

        return Get(layout);
    }
};
