#pragma once

#include <cassert>
#include "RenderDevice.h"
#include "VertexLayoutDesc.h"

class VertexLayoutCache {
private:
    graphics::RenderDevice* _device;

public:
    VertexLayoutCache(graphics::RenderDevice* device) : _device(device) {}

    graphics::VertexLayoutId Get(const graphics::VertexLayoutDesc& vld) {
        // TODO: actually cache
        graphics::VertexLayoutId vlId = _device->CreateVertexLayout(vld);
        assert(vlId);
        return vlId;
    }

    graphics::VertexLayoutId Pos3fNormal3fTex2f() {
        static graphics::VertexLayoutDesc layout{
        { {
                    graphics::VertexAttributeType::Float3, graphics::VertexAttributeUsage::Position,
                },
                {
                    graphics::VertexAttributeType::Float3, graphics::VertexAttributeUsage::Normal,
                },
                {
                    graphics::VertexAttributeType::Float2, graphics::VertexAttributeUsage::Texcoord0,
        } } };

        return Get(layout);
    }

    graphics::VertexLayoutId Pos3fNormal3f() {
        static graphics::VertexLayoutDesc layout{
        {{
                 graphics::VertexAttributeType::Float3, graphics::VertexAttributeUsage::Position,
             },
             {
                 graphics::VertexAttributeType::Float3, graphics::VertexAttributeUsage::Normal,
        }}};

        return Get(layout);
    }

    graphics::VertexLayoutId Pos3f() {
        static graphics::VertexLayoutDesc layout{
        {{
                 graphics::VertexAttributeType::Float3, graphics::VertexAttributeUsage::Position,
             },
            
        }};

        return Get(layout);
    }
};