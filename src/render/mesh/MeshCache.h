#pragma once

#include "Mesh.h"
#include "MeshNode.h"
#include "MeshImporter.h"
#include "RenderCache.h"

#include <vector>

struct MeshCachePolicy {
private:
    // this is in bytes, should be about ~20mb, 650k 32 byte vertices
    constexpr static uint32_t MAX_VERT_BUFF_SIZE = 650000 * 32;
    // also in bytes for now just same size as vert buff
    constexpr static uint32_t MAX_INDEX_BUFF_SIZE = MAX_VERT_BUFF_SIZE;

    gfx::TextureId vertBufferId;
    gfx::TextureId indexBufferId;

    uint32_t vertOffset{ 0 };
    uint32_t indexOffset{ 0 };

    gfx::RenderDevice* _device{ nullptr };

public:
    using CacheItemType = MeshPtr;
    using FileDataType = meshImport::MeshData;

    MeshCachePolicy(gfx::RenderDevice* device) : _device(device) {
        gfx::BufferDesc vBufDesc = gfx::BufferDesc::vbPersistent(MAX_VERT_BUFF_SIZE, "MeshSharedVB");

        gfx::BufferDesc iBufDesc = gfx::BufferDesc::ibPersistent(MAX_INDEX_BUFF_SIZE, "MeshSharedIB");

        vertBufferId = _device->AllocateBuffer(vBufDesc);
        indexBufferId = _device->AllocateBuffer(iBufDesc);
    }

    FileDataType LoadDataFromFile(const std::string& fpath) {
        return meshImport::LoadMeshDataFromFile(fpath);
    }

    CacheItemType ConstructCacheItem(FileDataType&& data) {
        auto meshGeom = UploadGetMeshsToGpu(data.nodes, data.geomData);
        return std::make_shared<Mesh>(std::move(data.nodes), std::move(meshGeom), std::move(data.boneInfo), std::move(data.gimt), std::move(data.tree));
    }

private:
    std::vector<MeshGeometry> UploadGetMeshsToGpu(const std::vector<MeshNode>& nodes, const std::vector<MeshGeometryData>& geomData) {
        std::vector<MeshGeometry> meshGeom;
        meshGeom.reserve(geomData.size());
        // todo: edit the actual mesh and update the 'meshidx' to allow 'instancing' to happen inside of an actual mesh
        // just using the fact that the meshidx itself is useless otherwise

        for (const MeshNode& node : nodes) {
            for (const MeshPart& part : node.meshParts) {
                const MeshGeometryData& data = geomData[part.meshIdx];
                if (data.indexCount() == 0 && data.vertexCount() == 0)
                    continue;
                assert(MAX_VERT_BUFF_SIZE > ((vertOffset + data.vertexCount()) * data.vertexLayout().stride()));
                assert(MAX_INDEX_BUFF_SIZE > ((indexOffset + data.indexCount()) * sizeof(uint32_t)));

                MeshGeomExistBuffer existBuffer;
                existBuffer.indexBuffer = indexBufferId;
                existBuffer.indexOffset = indexOffset;
                existBuffer.vertexBuffer = vertBufferId;
                existBuffer.vertexOffset = vertOffset;

                meshGeom.push_back({ _device, data, &existBuffer });
                meshGeom.back().meshMaterialId = part.matIdx;

                vertOffset += data.vertexCount();
                indexOffset += data.indexCount();
            }
        }
        return meshGeom;
    }
};

using MeshCache = RenderCache<MeshCachePolicy>;