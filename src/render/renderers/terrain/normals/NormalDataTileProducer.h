#pragma once

#include <memory>
#include "CPUNormalsDataTileProducer.h"
#include "DataTileProducer.h"
#include "GPUTileBuffer.h"
#include "RenderDevice.h"
#include "TerrainDataTile.h"
#include "TileCache.h"

class GPUNormalsDataTile : public TerrainDataTile {
public:
    GPUNormalsDataTile(const TerrainTileKey& key, const glm::uvec2& res)
        : TerrainDataTile(key, TerrainLayerType::Normalmap, res) {}
    GPUTileSlot<gfx::PixelFormat::RGBA32Float>* gpuData{nullptr};
};

class NormalDataTileProducer : public DataTileProducer, public DataTileSampler<GPUNormalsDataTile> {
private:
    using NormalmapGPUTileBuffer = GPUTileBuffer<gfx::PixelFormat::RGBA32Float>;
    using NormalmapGPUTileCache  = GPUTileCache<TerrainTileKey, gfx::PixelFormat::RGBA32Float>;
    using NormalmapGPUTileSlot   = NormalmapGPUTileBuffer::Slot;

private:
    CPUNormalDataTileProducer* _normalsProducer{nullptr};
    gfx::RenderDevice*         _device;

    std::map<TerrainTileKey, GPUNormalsDataTile*> _dataTiles;

    std::unique_ptr<NormalmapGPUTileBuffer> _gpuTileBuffer;
    std::unique_ptr<NormalmapGPUTileCache>  _gpuTileCache;

public:
    NormalDataTileProducer(gfx::RenderDevice* device, CPUNormalDataTileProducer* normalsProducer)
        : DataTileProducer(TerrainLayerType::Normalmap, normalsProducer->tileResolution)
        , _normalsProducer(normalsProducer)
        , _device(device) {
        _gpuTileBuffer.reset(new NormalmapGPUTileBuffer(_device, DataTileProducer::tileResolution.x, DataTileProducer::tileResolution.y, 128));
        _gpuTileCache.reset(new NormalmapGPUTileCache(_gpuTileBuffer.get(), [&](const TerrainTileKey& key, const NormalmapGPUTileSlot* slot) {
            auto it = _dataTiles.find(key);
            if (it != end(_dataTiles)) {
                GPUNormalsDataTile* tile = it->second;
                _dataTiles.erase(key);
                delete tile;
            }
        }));
    }
    ~NormalDataTileProducer() {}

    // DataTileProducer interface
    virtual GPUNormalsDataTile* GetTile(const TerrainQuadNode& node) final {
        auto it = _dataTiles.find(node.key);
        if (it == _dataTiles.end()) {
            return nullptr;
        } else {
            // touching cached data
            _gpuTileCache->find(node.key);
            return it->second;
        }
    }

    virtual void Update(const std::vector<const TerrainQuadNode*>& nodesInScene, const std::set<TerrainTileKey>& keysLeaving,
                        const std::set<TerrainTileKey>& keysEntering) final {

        for (const TerrainQuadNode* node : nodesInScene) {
            TerrainDataTile* tile = GetTile(*node);
            if (tile == nullptr) {
                NormalmapGPUTileSlot* gpuSlot = _gpuTileCache->find(node->key);

                if (gpuSlot == nullptr) {
                    // not cached, need cpu data

                    CPUNormalsDataTile* cpuElevationData = reinterpret_cast<CPUNormalsDataTile*>(_normalsProducer->GetTile(*node));
                    if (cpuElevationData == nullptr || cpuElevationData->cpuData == nullptr) {
                        // wait still waiting for cpu data to be generated
                        // TODO:: need to prod producer to generate the elevation data
                        continue;
                    } else {
                        bool result = _gpuTileCache->get(node->key, &gpuSlot);
                        dg_assert_nm(result == false);
                        _device->UpdateTexture(gpuSlot->texture, gpuSlot->slotIndex, cpuElevationData->cpuData->data.data());
                    }
                }

                dg_assert_nm(gpuSlot);
                GPUNormalsDataTile* elevationDataTile = new GPUNormalsDataTile(node->key, tileResolution);
                elevationDataTile->gpuData            = gpuSlot;

                _dataTiles.insert({node->key, elevationDataTile});
            }
        }
    }

    // DataTileSampler interface
    virtual GPUNormalsDataTile* FindTile(const TerrainTileKey& key) final {
        TerrainTileKey k  = key;
        auto           it = _dataTiles.find(k);
        while (k.lod != 0 && it == end(_dataTiles)) {
            k  = getParentKey(k);
            it = _dataTiles.find(k);
        }
        return it == _dataTiles.end() ? nullptr : it->second;
    }
};
