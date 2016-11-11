#pragma once

#include <memory>
#include <vector>
#include "BlockingQueue.h"
#include "DataTileProducer.h"
#include "ElevationDataTile.h"
#include "GPUTileBuffer.h"
#include "GenerateHeightmapTask.h"
#include "MeshGeometry.h"
#include "RenderDevice.h"
#include "TileCache.h"

class ElevationDataTileProducer : public DataTileProducer, public DataTileSampler<ElevationDataTile> {
private:
    using HeightmapGPUTileBuffer = GPUTileBuffer<gfx::PixelFormat::R32Float>;
    using HeightmapGPUTileCache  = GPUTileCache<TerrainTileKey, gfx::PixelFormat::R32Float>;
    using HeightmapGPUTileSlot   = HeightmapGPUTileBuffer::Slot;

    using HeightmapCPUTileBuffer = CPUTileBuffer<float>;
    using HeightmapCPUTileCache  = CPUTileCache<TerrainTileKey, float>;
    using HeightmapCPUTileSlot   = HeightmapCPUTileBuffer::Slot;

private:
    gfx::RenderDevice* _device{nullptr};

    std::unordered_map<TerrainTileKey, TaskPtr> _pendingTasks;

    std::unique_ptr<BlockingQueue<GenerateHeightmapTaskResults>> _generateHeightmapTaskOutput;

    std::map<TerrainTileKey, ElevationDataTile*> _dataTiles;
    std::unique_ptr<HeightmapGPUTileBuffer> _gpuTileBuffer;
    std::unique_ptr<HeightmapGPUTileCache>  _gpuTileCache;
    std::unique_ptr<HeightmapCPUTileBuffer> _cpuTileBuffer;
    std::unique_ptr<HeightmapCPUTileCache>  _cpuTileCache;

    const glm::uvec2              _tileResolution;
    std::unique_ptr<MeshGeometry> _tileGeometry;

public:
    ElevationDataTileProducer(gfx::RenderDevice* device, const glm::uvec2& tileResolution);
    ~ElevationDataTileProducer();

    virtual ElevationDataTile* GetTile(const TerrainQuadNode& node) override;
    virtual void Update(const std::vector<const TerrainQuadNode*>& nodesInScene) override;

private:
    void GenerateHeightmapRegion(const glm::vec2& regionCenter, const glm::vec2& regionSize, const glm::uvec2& resolution,
                                 std::function<float(float localX, float localY)> heightDelegate, std::vector<float>* data, float* max, float* min);
    virtual ElevationDataTile* FindTile(const TerrainTileKey& key) override;
    void dumpCachedHeightmapsToDisk();
};
