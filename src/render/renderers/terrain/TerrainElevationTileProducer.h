#pragma once

#include <array>
#include <atomic>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <noise/noise.h>
#include <queue>
#include <thread>
#include <unordered_set>
#include <vector>
#include "BlockingQueue.h"
#include "DGAssert.h"
#include "GPUTileBuffer.h"
#include "Log.h"
#include "Pool.h"
#include "RenderDevice.h"
#include "TerrainDataTile.h"
#include "TerrainElevationTile.h"
#include "TerrainQuadTree.h"
#include "TerrainTileProducer.h"
#include "TileCache.h"

class TerrainElevationTileProducer : public TerrainTileProducer {
private:
    using HeightmapGPUTileBuffer = GPUTileBuffer<gfx::PixelFormat::R32Float>;
    using HeightmapGPUTileCache  = GPUTileCache<TerrainTileKey, gfx::PixelFormat::R32Float>;
    using HeightmapGPUTileSlot   = HeightmapGPUTileBuffer::Slot;

    using HeightmapCPUTileBuffer = CPUTileBuffer<float>;
    using HeightmapCPUTileCache  = CPUTileCache<TerrainTileKey, float>;
    using HeightmapCPUTileSlot   = HeightmapCPUTileBuffer::Slot;

    using TerrainElevationTileCache = LRUCache<TerrainTileKey, TerrainElevationTile*>;

    struct Job {
        Job(const TerrainQuadNode* node) : node(node) {}

        const TerrainQuadNode* const node; // (TODO) should extract what is needed off node instead of using ptr
        TerrainElevationTile*        tile{nullptr};
        bool                         isGpuSlotUnused{true};
    };

private:
    gfx::RenderDevice* _device{nullptr};

    std::unordered_set<TerrainTileKey> _pendingKeys;
    BlockingQueue<Job*>                _pendingJobs;
    BlockingQueue<Job*>                _completedJobs;

    std::thread       _worker;
    std::atomic<bool> _backgroundThreadActive{true};

    std::unique_ptr<Pool<Job>>                  _jobPool;
    std::unique_ptr<Pool<TerrainElevationTile>> _terrainElevationTilePool;
    std::unique_ptr<TerrainElevationTileCache>  _TerrainDataTileCache;

    std::unique_ptr<HeightmapGPUTileBuffer> _gpuTileBuffer;
    std::unique_ptr<HeightmapGPUTileCache>  _gpuTileCache;
    std::unique_ptr<HeightmapCPUTileBuffer> _cpuTileBuffer;
    std::unique_ptr<HeightmapCPUTileCache>  _cpuTileCache;

    const glm::uvec2 _tileResolution;

public:
    TerrainElevationTileProducer(gfx::RenderDevice* device, const glm::uvec2& tileResolution)
        : TerrainTileProducer(TerrainLayerType::Heightmap), _device(device), _tileResolution(tileResolution) {
        _jobPool.reset(new Pool<Job>(32));
        _terrainElevationTilePool.reset(new Pool<TerrainElevationTile>(256));
        _TerrainDataTileCache.reset(
            new TerrainElevationTileCache(256, [&](const TerrainTileKey& key, TerrainElevationTile* tile) { _terrainElevationTilePool->release(tile); }));

        _gpuTileBuffer.reset(new HeightmapGPUTileBuffer(_device, _tileResolution.x, _tileResolution.y, 32));
        _gpuTileCache.reset(
            new HeightmapGPUTileCache(_gpuTileBuffer.get(), [&](const TerrainTileKey& key, const HeightmapGPUTileSlot* slot) { _TerrainDataTileCache->evict(key); }));
        _cpuTileBuffer.reset(new HeightmapCPUTileBuffer(_tileResolution.x, _tileResolution.y, _gpuTileBuffer->capacity() * 2));
        _cpuTileCache.reset(
            new HeightmapCPUTileCache(_cpuTileBuffer.get(), [&](const TerrainTileKey& key, const HeightmapCPUTileSlot* slot) { _TerrainDataTileCache->evict(key); }));
        _worker = std::thread([&]() {
            Job* job = nullptr;
            while (_backgroundThreadActive) {
                _pendingJobs.dequeue(&job);

                const TerrainQuadNode* node    = job->node;
                TerrainQuadTree*       terrain = node->terrain;

                TerrainElevationTile* tile = _terrainElevationTilePool->construct(node->key);
                dg_assert_nm(tile != nullptr);
                tile->transform = glm::translate(terrain->transform(), terrain->localToWorld(node->local));

                // first, check if we have the tile data cached
                tile->gpuSlot = _gpuTileCache->find(node->key);
                tile->cpuSlot = _cpuTileCache->find(node->key);

                // record whether the gpuSlot will have valid or invalid/no data
                job->isGpuSlotUnused = tile->gpuSlot == nullptr;

                // get a gpuSlot to use
                std::pair<bool, HeightmapGPUTileSlot*> gpuResultPair = _gpuTileCache->get(node->key);
                dg_assert_nm(gpuResultPair.first == false && gpuResultPair.second != nullptr);
                tile->gpuSlot = gpuResultPair.second;

                if (job->isGpuSlotUnused && !tile->cpuSlot) {
                    // if we have no data cached for this key, need to generate
                    std::pair<bool, HeightmapCPUTileSlot*> cpuResultPair = _cpuTileCache->get(node->key);
                    dg_assert_nm(cpuResultPair.first == false && cpuResultPair.second != nullptr);
                    tile->cpuSlot = cpuResultPair.second;

                    auto getHeightForLocalCoord = [&terrain](float localX, float localY) {
                        glm::dvec3                 world = terrain->localToWorld({localX, localY});
                        noise::module::RidgedMulti mountain;
                        mountain.SetSeed(32);
                        mountain.SetFrequency(0.05);
                        mountain.SetOctaveCount(8);
                        return mountain.GetValue(world.x * 0.01f, world.y * 0.01f, world.z * 0.01f);
                    };

                    glm::vec2  regionCenter = {node->local.x, node->local.y};
                    glm::vec2  regionSize   = {node->size, node->size};
                    glm::uvec2 resolution   = {256, 256};

                    float max = 0, min = 0;
                    tile->cpuSlot->data.clear();
                    GenerateHeightmapRegion(regionCenter, regionSize, resolution, getHeightForLocalCoord, &tile->cpuSlot->data, &max, &min);
                }

                job->tile = tile;
                _completedJobs.enqueue(job);
            }
        });
    }

    ~TerrainElevationTileProducer() {
        _backgroundThreadActive = false;
        _worker.join();
    }

    TerrainDataTile* GetTile(const TerrainQuadNode& node) { return FindTile(node.key); }

    bool GetTiles(const std::vector<const TerrainQuadNode*>& nodes, std::vector<TerrainDataTile*>* outputVector) {
        std::vector<Job*> jobsToQueue;
        for (const TerrainQuadNode* node : nodes) {
            TerrainDataTile* tile = GetTile(*node);
            if (!tile) {
                if (_pendingKeys.count(node->key) > 0)
                    continue;

                // tile is not cached and no jobs for current key
                Job* job = _jobPool->construct(node);
                if (job == nullptr) {
                    continue;
                }

                _pendingKeys.insert(node->key);
                jobsToQueue.push_back(job);
            } else {
                dg_assert_nm(tile > 0);
                outputVector->push_back(tile);
            }
        }

        _pendingJobs.enqueueAll(jobsToQueue);
        return true;
    }

    virtual void Update() {
        std::vector<Job*> completed;
        _completedJobs.flush(&completed);

        for (Job* job : completed) {
            const TerrainQuadNode* node = job->node;
            TerrainElevationTile*  tile = job->tile;
            dg_assert_nm(tile->gpuSlot && tile->cpuSlot);
            if (job->isGpuSlotUnused) {
                _device->UpdateTexture(tile->gpuSlot->texture, tile->gpuSlot->slotIndex, tile->cpuSlot->data.data());
            }
            _TerrainDataTileCache->put(node->key, tile);
            _jobPool->release(job);
            _pendingKeys.erase(node->key);
        }
    }

private:
    /*
     @param region_center Center point or region to generate coordinates from
     @param region_size Size of region to generate
     @param resolution Resolution to sample for generating heightmap values
     @param heightmap_generator Function to call to generate a heightmap value for a give local coordinate
     @param data Output buffer to store heightmap values in
     @param max Output parameter to store max height value
     @param min Output parameter to store min height value
     */
    void GenerateHeightmapRegion(const glm::vec2& regionCenter, const glm::vec2& regionSize, const glm::uvec2& resolution,
                                 std::function<float(float localX, float localY)> heightDelegate, std::vector<float>* data, float* max, float* min) {

        *max = std::numeric_limits<float>::min();
        *min = std::numeric_limits<float>::max();

        glm::vec2 halfSize = regionSize / 2.f;
        float     dx       = regionSize.x / (float)(resolution.x - 1);
        float     dy       = regionSize.y / (float)(resolution.y - 1);

        for (uint32_t i = 0; i < resolution.y; ++i) {
            for (uint32_t j = 0; j < resolution.x; ++j) {
                float x = regionCenter.x - halfSize.x + (j * dx);
                float y = regionCenter.y + halfSize.y - (i * dy);

                float val = heightDelegate(x, y);

                if (val > *max) {
                    *max = val;
                } else if (val < *min) {
                    *min = val;
                }

                data->push_back(val);
            }
        }
    }

    TerrainDataTile* FindTile(const TerrainTileKey& key) {
        TerrainElevationTile* tile = nullptr;
        _TerrainDataTileCache->get(key, &tile);
        return tile;
    }
};
