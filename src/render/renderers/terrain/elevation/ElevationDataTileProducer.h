#pragma once

#include <memory>
#include <vector>
#include "BlockingQueue.h"
#include "CPUElevationDataTileProducer.h"
#include "DataTileProducer.h"
#include "ElevationDataTile.h"
#include "GPUTileBuffer.h"
#include "GenerateHeightmapTask.h"
#include "MeshGeometry.h"
#include "RenderDevice.h"
#include "TileCache.h"

class ElevationDataTileProducer : public DataTileProducer, public DataTileSampler<GPUElevationDataTile> {
private:
    using HeightmapGPUTileBuffer = GPUTileBuffer<gfx::PixelFormat::R32Float>;
    using HeightmapGPUTileCache  = GPUTileCache<TerrainTileKey, gfx::PixelFormat::R32Float>;
    using HeightmapGPUTileSlot   = HeightmapGPUTileBuffer::Slot;

    using HeightmapCPUTileBuffer = CPUTileBuffer<float>;
    using HeightmapCPUTileCache  = CPUTileCache<TerrainTileKey, float>;
    using HeightmapCPUTileSlot   = HeightmapCPUTileBuffer::Slot;

private:
    gfx::RenderDevice* _device{nullptr};

    std::map<TerrainTileKey, GPUElevationDataTile*> _dataTiles;
    std::unique_ptr<HeightmapGPUTileBuffer> _gpuTileBuffer;
    std::unique_ptr<HeightmapGPUTileCache>  _gpuTileCache;

    CPUElevationDataTileProducer* _dataProducer;
    std::unique_ptr<MeshGeometry> _tileGeometry;

public:
    ElevationDataTileProducer(gfx::RenderDevice* device, CPUElevationDataTileProducer* cpuElevationDataTileProducer);
    ~ElevationDataTileProducer();

    // DataTileProducer interface
    virtual GPUElevationDataTile* GetTile(const TerrainQuadNode& node) final;
    virtual void Update(const std::vector<const TerrainQuadNode*>& nodesInScene, const std::set<TerrainTileKey>& keysLeaving,
                        const std::set<TerrainTileKey>& keysEntering) final;

    // DataTileSampler interface
    virtual GPUElevationDataTile* FindTile(const TerrainTileKey& key) final;

private:
    void GenerateHeightmapRegion(const glm::vec2& regionCenter, const glm::vec2& regionSize, const glm::uvec2& resolution,
                                 std::function<float(float localX, float localY)> heightDelegate, std::vector<float>* data, float* max, float* min);
};
