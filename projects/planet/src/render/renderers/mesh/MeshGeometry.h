#pragma once

#include <vector>
#include "DrawCall.h"
#include "MeshGeometryData.h"
#include "RenderDevice.h"
#include "ResourceTypes.h"
#include "StateGroupEncoder.h"
#include <memory>

struct MeshGeomExistBuffer {
    gfx::BufferId       vertexBuffer{ 0 };
    gfx::BufferId       indexBuffer{ 0 };
    uint32_t            vertexOffset{ 0 };
    uint32_t            indexOffset{ 0 };
};

class MeshGeometry {
private:
    gfx::RenderDevice* _device{nullptr};

    gfx::BufferId       vertexBuffer{0};
    gfx::BufferId       indexBuffer{0};
    gfx::VertexLayoutId vertexlayout{0};
    uint32_t            vertexCount{0};
    uint32_t            vertexOffset{0};
    uint32_t            indexCount{0};
    uint32_t            indexOffset{0};

    size_t vertexlayoutStride{ 0 };

    gfx::DrawCall             m_drawCall;
    std::unique_ptr<const gfx::StateGroup> _stateGroup;

private:
    void SetStateGroupAndDrawCall() {
        if (indexCount > 0) {
            m_drawCall.type = gfx::DrawCall::Type::Indexed;
            m_drawCall.primitiveCount = indexCount;
            m_drawCall.baseVertexOffset = vertexOffset;
            m_drawCall.startOffset = indexOffset;
        }
        else {
            m_drawCall.type = gfx::DrawCall::Type::Arrays;
            m_drawCall.startOffset = vertexOffset;
            m_drawCall.primitiveCount = vertexCount;
        }

        gfx::StateGroupEncoder encoder;
        encoder.Begin();
        encoder.SetVertexBuffer(vertexBuffer);
        encoder.SetIndexBuffer(indexBuffer);
        encoder.SetVertexLayout(vertexlayout);
        _stateGroup.reset(encoder.End());
    }

public:
    // temp: probly should move this
    uint32_t            meshMaterialId{ 0 };

    MeshGeometry(gfx::RenderDevice* device, const MeshGeometryData& meshData, const MeshGeomExistBuffer* existingBufData = nullptr) : _device(device) {

        // vertex layout *should* be fine to recreate every time as it should be cached
        gfx::VertexLayoutDesc vld = meshData.vertexLayout();
        vertexlayout              = _device->CreateVertexLayout(vld);

        // this will blow up if the layouts are not all the same currently
        assert(vertexlayoutStride == 0 || vertexlayoutStride == vld.stride());
        vertexlayoutStride = vld.stride();

        vertexCount = meshData.vertexCount();
        indexCount = meshData.indexCount();

        std::vector<uint8_t> interleavedVertexData(vertexCount * vertexlayoutStride);

        meshData.interleave(interleavedVertexData.data());
        
        if (existingBufData == nullptr) {
            vertexBuffer = _device->AllocateBuffer(gfx::BufferDesc::vbPersistent(vld.stride() * vertexCount, "meshGeomVB"), interleavedVertexData.data());
            if (indexCount > 0) {
                indexBuffer = _device->AllocateBuffer(gfx::BufferDesc::ibPersistent(sizeof(uint32_t) * indexCount, "meshGeomIB"), meshData.indices.data());
            }

        }
        else {
            vertexBuffer = existingBufData->vertexBuffer;
            vertexOffset = existingBufData->vertexOffset;
            indexBuffer = existingBufData->indexBuffer;
            indexOffset = existingBufData->indexOffset;
            uint8_t* mem = _device->MapMemory(vertexBuffer, gfx::BufferAccess::WriteNoOverwrite);
            memcpy(&mem[vertexOffset * vertexlayoutStride], interleavedVertexData.data(), vertexCount * vertexlayoutStride);
            _device->UnmapMemory(vertexBuffer);

            if (meshData.hasIndices()) {
                uint8_t* mem = _device->MapMemory(indexBuffer, gfx::BufferAccess::WriteNoOverwrite);
                memcpy(&mem[indexOffset * sizeof(uint32_t)], meshData.indices.data(), indexCount * sizeof(uint32_t));
                _device->UnmapMemory(indexBuffer);

            }
        }

        SetStateGroupAndDrawCall();
    }

    const gfx::DrawCall& drawCall() { return m_drawCall; }

    const gfx::StateGroup* stateGroup() { return _stateGroup.get(); }
};
