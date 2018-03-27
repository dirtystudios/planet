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

class CPUElevationDataTileProducer : public DataTileProducer, public DataTileSampler<CPUElevationDataTile> {
private:
    using HeightmapCPUTileBuffer = CPUTileBuffer<float>;
    using HeightmapCPUTileCache  = CPUTileCache<TerrainTileKey, float>;
    using HeightmapCPUTileSlot   = HeightmapCPUTileBuffer::Slot;

private:
    std::unordered_map<TerrainTileKey, TaskPtr> _pendingTasks;

    std::unique_ptr<BlockingQueue<GenerateHeightmapTaskResults>> _generateHeightmapTaskOutput;

    std::map<TerrainTileKey, CPUElevationDataTile*> _dataTiles;
    std::unique_ptr<HeightmapCPUTileBuffer> _cpuTileBuffer;
    std::unique_ptr<HeightmapCPUTileCache>  _cpuTileCache;

public:
    CPUElevationDataTileProducer(const glm::uvec2& tileResolution);
    ~CPUElevationDataTileProducer() {}

    // DataTileProducer interface
    virtual CPUElevationDataTile* GetTile(const TerrainQuadNode& node) final;
    virtual void Update(const std::vector<const TerrainQuadNode*>& nodesInScene, const std::set<TerrainTileKey>& keysLeaving,
                        const std::set<TerrainTileKey>& keysEntering) final;

    // DataTileSampler interface
    virtual CPUElevationDataTile* FindTile(const TerrainTileKey& key) final;

private:
    void dumpCachedHeightmapsToDisk();
};
