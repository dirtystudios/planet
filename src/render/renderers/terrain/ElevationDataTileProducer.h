#pragma once

#include <array>
#include <atomic>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <noise/noise.h>
#include <thread>
#include <unordered_set>
#include <vector>
#include "BlockingQueue.h"
#include "ConsoleCommands.h"
#include "DGAssert.h"
#include "DMath.h"
#include "File.h"
#include "GPUTileBuffer.h"
#include "Image.h"
#include "Log.h"
#include "MeshGeneration.h"
#include "Pool.h"
#include "RenderDevice.h"
#include "TaskScheduler.h"
#include "TerrainDataTile.h"
#include "TerrainElevationTile.h"
#include "TerrainQuadTree.h"
#include "DataTileProducer.h"
#include "TileCache.h"

class GenerateHeightmapTaskResults {
public:
    GenerateHeightmapTaskResults(const TerrainTileKey& key) : key(key) {}

    TerrainTileKey     key;
    double             min{std::numeric_limits<double>::max()};
    double             max{std::numeric_limits<double>::min()};
    std::vector<float> data;
};

class GenerateHeightmapTask : public Task {
private:
    GenerateHeightmapTaskResults _results;

    const dm::Rect3Dd                _region;
    const glm::uvec2                 _resolution;
    const noise::module::RidgedMulti _mountain;

    BlockingQueue<GenerateHeightmapTaskResults>* _outputQueue{nullptr};

public:
    GenerateHeightmapTask(const TerrainTileKey& key, const dm::Rect3Dd& region, const glm::uvec2& resolution, BlockingQueue<GenerateHeightmapTaskResults>* outputQueue)
        : _results({key}), _region(region), _resolution(resolution), _outputQueue(outputQueue) {
        noise::module::RidgedMulti mountain;
        mountain.SetSeed(32);
        mountain.SetFrequency(0.05);
        mountain.SetOctaveCount(8);
    }

    virtual void execute() final {
        double dx = _region.width() / (double)(_resolution.x - 1);
        double dy = _region.height() / (double)(_resolution.y - 1);

        for (uint32_t i = 0; i < _resolution.y; ++i) {
            double     t1       = dy * i / _region.height();
            glm::dvec3 rowStart = dm::lerp(_region.bl(), _region.tl(), t1);
            glm::dvec3 rowEnd   = dm::lerp(_region.br(), _region.tr(), t1);

            for (uint32_t j = 0; j < _resolution.x; ++j) {
                double     t2     = dx * j / _region.width();
                glm::dvec3 sample = dm::lerp(rowStart, rowEnd, t2);
                sample *= 0.01f;
                double val = dm::clamp(_mountain.GetValue(sample.x, sample.y, sample.z), -1.0, 1.0);

                if (val > _results.max) {
                    _results.max = val;
                } else if (val < _results.min) {
                    _results.min = val;
                }

                _results.data.push_back(val);
            }
        }
        _outputQueue->enqueue(_results);
    }
};

class ElevationDataTile : public TerrainDataTile {
public:
    ElevationDataTile(const TerrainTileKey& key) : TerrainDataTile(key, TerrainLayerType::Heightmap) {}

    CPUTileSlot<float>*                      cpuData{nullptr};
    GPUTileSlot<gfx::PixelFormat::R32Float>* gpuData{nullptr};
    MeshGeometry*                            geometry{nullptr};
    ConstantBuffer*                          perTileConstants{nullptr};
    std::unique_ptr<const gfx::StateGroup>   stateGroup;
    gfx::DrawItemPtr                         drawItem;
};

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
    ElevationDataTileProducer(gfx::RenderDevice* device, const glm::uvec2& tileResolution)
        : DataTileProducer(TerrainLayerType::Heightmap), _device(device), _tileResolution(tileResolution) {

        config::ConsoleCommands::getInstance().RegisterCommand("dumphm", [&](const std::vector<std::string>& params) -> std::string {
            this->dumpCachedHeightmapsToDisk();
            return "success";
        });

        _gpuTileBuffer.reset(new HeightmapGPUTileBuffer(_device, _tileResolution.x, _tileResolution.y, 128));
        _gpuTileCache.reset(new HeightmapGPUTileCache(_gpuTileBuffer.get(), [&](const TerrainTileKey& key, const HeightmapGPUTileSlot* slot) {
            auto it = _dataTiles.find(key);
            if (it != end(_dataTiles)) {
                ElevationDataTile* tile = it->second;
                _dataTiles.erase(key);
                delete tile;
            }

        }));
        _cpuTileBuffer.reset(new HeightmapCPUTileBuffer(_tileResolution.x, _tileResolution.y, _gpuTileBuffer->capacity() * 2));
        _cpuTileCache.reset(new HeightmapCPUTileCache(_cpuTileBuffer.get(), [&](const TerrainTileKey& key, const HeightmapCPUTileSlot* slot) {
            auto it = _dataTiles.find(key);
            if (it != end(_dataTiles)) {
                ElevationDataTile* tile = it->second;
                tile->cpuData           = nullptr;
            }

        }));
        _generateHeightmapTaskOutput.reset(new BlockingQueue<GenerateHeightmapTaskResults>());

        MeshGeometryData geometryData;
        dgen::GenerateGrid(glm::vec3(0, 0, 0), {2, 2}, _tileResolution, &geometryData);
        _tileGeometry.reset(new MeshGeometry(_device, geometryData));
    }

    ~ElevationDataTileProducer() {}

    ElevationDataTile* GetTile(const TerrainQuadNode& node) { return FindTile(node.key); }

    virtual void Update(const std::vector<const TerrainQuadNode*>& nodesInScene) {
        // process arrivals
        std::vector<GenerateHeightmapTaskResults> completed;
        _generateHeightmapTaskOutput->flush(&completed);

        for (const GenerateHeightmapTaskResults& results : completed) {
            dg_assert_nm(_dataTiles.find(results.key) == _dataTiles.end());

            ElevationDataTile* elevationDataTile = new ElevationDataTile(results.key);

            dg_assert_nm(_cpuTileCache->get(results.key, &elevationDataTile->cpuData) == false);
            dg_assert_nm(_gpuTileCache->get(results.key, &elevationDataTile->gpuData) == false);
            dg_assert_nm(elevationDataTile->cpuData && elevationDataTile->gpuData);

            elevationDataTile->cpuData->data = results.data;
            _device->UpdateTexture(elevationDataTile->gpuData->texture, elevationDataTile->gpuData->slotIndex, elevationDataTile->cpuData->data.data());

            elevationDataTile->geometry = _tileGeometry.get();

            _dataTiles.insert({results.key, elevationDataTile});
            _pendingTasks.erase(results.key);
        }

        // push tasks
        std::vector<TaskPtr> tasksToQueue;

        for (const TerrainQuadNode* node : nodesInScene) {
            TerrainDataTile* tile = GetTile(*node);
            // todo: touch cached data to lru it up
            if (tile == nullptr) {
                if (_pendingTasks.find(node->key) != end(_pendingTasks)) {
                    continue;
                }

                HeightmapGPUTileSlot* gpuSlot = _gpuTileCache->find(node->key);
                HeightmapCPUTileSlot* cpuSlot = _cpuTileCache->find(node->key);
                if (!gpuSlot && !cpuSlot) {
                    TaskPtr task = std::make_shared<GenerateHeightmapTask>(node->key, node->sampleRect, _tileResolution, _generateHeightmapTaskOutput.get());
                    _pendingTasks.emplace(node->key, task);
                    tasksToQueue.push_back(task);
                } else {
                    if (cpuSlot) {
                        // TODO make some tiles
                        dg_assert_nm(_dataTiles.find(node->key) == _dataTiles.end());

                        ElevationDataTile* elevationDataTile = new ElevationDataTile(node->key);
                        elevationDataTile->cpuData           = cpuSlot;
                        dg_assert_nm(_gpuTileCache->get(node->key, &elevationDataTile->gpuData) == false);
                        dg_assert_nm(elevationDataTile->cpuData && elevationDataTile->gpuData);

                        _device->UpdateTexture(elevationDataTile->gpuData->texture, elevationDataTile->gpuData->slotIndex, elevationDataTile->cpuData->data.data());

                        elevationDataTile->geometry = _tileGeometry.get();

                        _dataTiles.insert({node->key, elevationDataTile});

                    } else {
                        // dont do anything, unless we come up with a reason to need to cpuData as well
                    }
                }
            }
        }

        if (tasksToQueue.size() > 0) {
            scheduler()->queue()->enqueueAll(tasksToQueue);
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

    ElevationDataTile* FindTile(const TerrainTileKey& key) {
        TerrainTileKey k  = key;
        auto           it = _dataTiles.find(k);
        //        while(k.lod != 0 && it == end(_dataTiles)) {
        //            k = getParentKey(k);
        //            it = _dataTiles.find(k);
        //        }

        return it == _dataTiles.end() ? nullptr : it->second;
    }

    void dumpCachedHeightmapsToDisk() {
        // copy cache
        std::vector<HeightmapCPUTileCache::CacheEntry> cacheSnapshot;
        _cpuTileCache->copyContents(cacheSnapshot);

        std::string          rootDir = fs::GetProcessDirectory();
        std::vector<uint8_t> data;
        data.reserve(_tileResolution.x * _tileResolution.y);
        for (const HeightmapCPUTileCache::CacheEntry& cacheEntry : cacheSnapshot) {
            data.clear();

            TerrainTileKey        key     = cacheEntry.first;
            HeightmapCPUTileSlot* cpuSlot = cacheEntry.second;
            TerrainTileKey        parent  = getParentKey(key);
            std::string           dirPath = rootDir + "heightmap_dump/terrain_" + std::to_string(key.tid) + "/lod_" + std::to_string(key.lod) + "/" + asFilename(parent);
            std::string           fpath   = dirPath + "/" + asFilename(key) + ".png";
            dg_assert_nm(fs::mkdirs(dirPath));
            std::transform(begin(cpuSlot->data), end(cpuSlot->data), std::back_inserter(data), [&](float d) { return static_cast<uint8_t>((d + 1.0) * 127.5f); });
            dg_assert_nm(dimg::WriteImageToFile(fpath.c_str(), _tileResolution.x, _tileResolution.y, dimg::PixelFormat::R8Unorm, data.data()));
        }
    }
};
