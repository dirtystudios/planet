#pragma once

#include <vector>
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

    std::vector<gfx::DrawCall>             m_drawCalls;
    std::unique_ptr<const gfx::StateGroup> _stateGroup;

public:
    MeshGeometry(gfx::RenderDevice* device, const std::vector<MeshGeometryData>& meshDatas) : _device(device) {
        assert(meshDatas.size() > 0);
        // todo: assuming all the same layout's for now
		
		dg_assert(std::find_if_not(std::begin(meshDatas), std::end(meshDatas), [&](const MeshGeometryData& v) { return v.vertexLayout() == meshDatas[0].vertexLayout(); })  == std::end(meshDatas), "not all same layout");
        gfx::VertexLayoutDesc vld = meshDatas[0].vertexLayout();
        vertexlayout              = _device->CreateVertexLayout(vld);
        size_t vertexlayoutStride = vld.stride();

        uint32_t vertexCount = 0;
        uint32_t indexCount  = 0;

        for (const MeshGeometryData& geomData : meshDatas) {
            vertexCount += geomData.vertexCount();
            indexCount += geomData.indexCount();
        }

        std::vector<uint8_t> interleavedVertexData(vertexCount * vertexlayoutStride);
        std::vector<uint8_t> indicesData(indexCount * sizeof(uint32_t));

        uint32_t voffset = 0;
        uint32_t ioffset = 0;
        for (const MeshGeometryData& geomData : meshDatas) {
            geomData.interleave(&interleavedVertexData[voffset]);
            if (geomData.hasIndices())
                memcpy(&indicesData[ioffset], geomData.indices.data(), geomData.indexCount() * sizeof(uint32_t));
            voffset += (geomData.vertexCount() * vertexlayoutStride);
            ioffset += (geomData.indexCount() * sizeof(uint32_t));
        }

        vertexBuffer = _device->AllocateBuffer(gfx::BufferDesc::vbPersistent(vld.stride() * vertexCount), interleavedVertexData.data());
        if (indexCount > 0) {
            indexBuffer = _device->AllocateBuffer(gfx::BufferDesc::ibPersistent(sizeof(uint32_t) * indexCount), indicesData.data());
        }

        uint32_t indexOffset  = 0;
        uint32_t vertexOffset = 0;
        for (const MeshGeometryData& geomData : meshDatas) {
            gfx::DrawCall dc;
            if (geomData.hasIndices()) {
                dc.type             = gfx::DrawCall::Type::Indexed;
                dc.primitiveCount   = geomData.indexCount();
                dc.baseVertexOffset = vertexOffset;
                dc.startOffset      = indexOffset;
                indexOffset += geomData.indexCount();
            } else {
                dc.type           = gfx::DrawCall::Type::Arrays;
                dc.startOffset    = 0;
                dc.primitiveCount = geomData.vertexCount();
            }
            vertexOffset += geomData.vertexCount();

            m_drawCalls.push_back(std::move(dc));
        }

        // lets assume 1 stategroup for now
        gfx::StateGroupEncoder encoder;
        encoder.Begin();
        encoder.SetVertexBuffer(vertexBuffer);
        encoder.SetIndexBuffer(indexBuffer);
        encoder.SetVertexLayout(vertexlayout);
        _stateGroup.reset(encoder.End());
    }

    const std::vector<gfx::DrawCall>& drawCalls() { return m_drawCalls; }

    const gfx::StateGroup* stateGroup() { return _stateGroup.get(); }
};
