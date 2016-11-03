#pragma once

#include "DrawCall.h"
#include "MeshGeometryData.h"
#include "RenderDevice.h"
#include "ResourceTypes.h"
#include "StateGroupEncoder.h"

class MeshGeometry {
private:
    gfx::RenderDevice* _device{nullptr};

    uint32_t            id{0};
    gfx::BufferId       vertexBuffer{0};
    gfx::BufferId       indexBuffer{0};
    gfx::VertexLayoutId vertexlayout{0};
    uint32_t            vertexCount{0};
    uint32_t            vertexOffset{0};
    uint32_t            indexCount{0};
    uint32_t            indexOffset{0};

    gfx::DrawCall                          dc;
    std::unique_ptr<const gfx::StateGroup> _stateGroup;

public:
    MeshGeometry(gfx::RenderDevice* device, const MeshGeometryData& data) : _device(device) {
        gfx::VertexLayoutDesc vld = data.vertexLayout();
        vertexlayout              = _device->CreateVertexLayout(vld);

        std::vector<uint8_t> interleavedVertexData;
        interleavedVertexData.resize(data.vertexCount() * vld.stride());
        data.interleave(interleavedVertexData.data());
        vertexBuffer = _device->AllocateBuffer(gfx::BufferDesc::vbPersistent(vld.stride() * data.vertexCount()), interleavedVertexData.data());
        if (data.hasIndices()) {
            indexBuffer = _device->AllocateBuffer(gfx::BufferDesc::ibPersistent(sizeof(uint32_t) * data.indexCount()));
        }
        vertexCount = data.vertexCount();
        indexCount  = data.indexCount();

        dc.type           = data.hasIndices() ? gfx::DrawCall::Type::Indexed : gfx::DrawCall::Type::Arrays;
        dc.offset         = 0;
        dc.primitiveCount = data.hasIndices() ? indexCount : vertexCount;

        gfx::StateGroupEncoder encoder;
        encoder.Begin();
        encoder.SetVertexBuffer(vertexBuffer);
        encoder.SetIndexBuffer(indexBuffer);
        encoder.SetVertexLayout(vertexlayout);
        _stateGroup.reset(encoder.End());
    }

    const gfx::DrawCall& drawCall() { return dc; }

    const gfx::StateGroup* stateGroup() { return _stateGroup.get(); }
};
