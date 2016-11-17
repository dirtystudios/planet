#pragma once

#include <memory>
#include <thread>
#include "CPUElevationDataTileProducer.h"
#include "DataTileProducer.h"
#include "GPUTileBuffer.h"
#include "RenderDevice.h"
#include "Task.h"
#include "TaskScheduler.h"
#include "TerrainDataTile.h"
#include "TileCache.h"

static const std::string kEDPChannel = "tileproducer.cpunormals";
#define EDPLog_W(fmt, ...) LOG(Log::Level::Warn, kEDPChannel, fmt, ##__VA_ARGS__)

struct GenerateNormalmapTaskResults {
    TerrainTileKey         key;
    std::vector<glm::vec4> data;
};

struct GenerateNormalmapTask : public Task {
private:
    const CPUElevationDataTile*                  elevationData;
    const TerrainTileKey                         key;
    BlockingQueue<GenerateNormalmapTaskResults>* _outputQueue;

public:
    GenerateNormalmapTask(const TerrainTileKey& key, const CPUElevationDataTile* cpuElevationData, BlockingQueue<GenerateNormalmapTaskResults>* outputQueue)
        : elevationData(cpuElevationData)
        , key(key)
        , _outputQueue(outputQueue) {}

    virtual void execute() final {
        //        std::this_thread::sleep_for (std::chrono::seconds(1));
        const float*           heightmapData = elevationData->cpuData->data.data();
        const glm::uvec2&      resolution    = elevationData->resolution;
        std::vector<glm::vec4> data(resolution.x * resolution.y);
        for (uint32_t i = 0; i < resolution.x; ++i) {
            for (uint32_t j = 0; j < resolution.y; ++j) {
                float tl = heightmapData[getIndex(i - 1, j - 1)];
                float t  = heightmapData[getIndex(i - 1, j)];
                float tr = heightmapData[getIndex(i - 1, j + 1)];
                float r  = heightmapData[getIndex(i, j + 1)];
                float br = heightmapData[getIndex(i + 1, j + 1)];
                float b  = heightmapData[getIndex(i + 1, j)];
                float bl = heightmapData[getIndex(i + 1, j - 1)];
                float l  = heightmapData[getIndex(i, j - 1)];

                // Note(eugene): not sure how to actually do this correctly
                //
                // There is an issue that depending on the terrain size or heightmap we set, the z
                // value chosen will be proportionally different that the x and y values that are
                // be computed. By scaling the x and y values by this term, x and y values will be
                // proportionally similar to the z value no matter the terrain size or resolution.
                //
                float scale = 1.0; //((float)resolution.x / (float)(size.x * pow(2, key.lod))) / 0.032f;

                // This needs to scale with region LoD to keep the ratio between size of z value and
                // size of x/y values similar. If we don't, the z value will dominate more an more in
                // the normal as we go to higher LoD regions.
                float z = 1.f / pow(2, (key.lod));

                float x = -((br - bl) + (2.f * (r - l)) + (tr - tl));
                float y = -((tl - bl) + (2.f * (t - b)) + (tr - br));

                data[getIndex(i, j)] = glm::normalize(glm::vec4(scale * x, scale * y, z, 0));
            }
        }
        _outputQueue->enqueue({key, data});
    }

private:
    // convert from 2D index to 1D index, clamping to edges (ex. i=-1 => i=0)
    int32_t getIndex(int32_t i, int32_t j) {
        const glm::uvec2& resolution = elevationData->resolution;
        int32_t           k          = (i >= (int32_t)resolution.x) ? resolution.x - 1 : ((i < 0) ? 0 : i);
        int32_t           p          = (j >= (int32_t)resolution.y) ? resolution.y - 1 : ((j < 0) ? 0 : j);
        // LOG_D("%d %d %d -> %d %d", j >= resolution.y, i, j, k, p);
        return k * resolution.y + p;
    };
};

class CPUNormalsDataTile : public TerrainDataTile {
public:
    CPUNormalsDataTile(const TerrainTileKey& key, const glm::uvec2& res)
        : TerrainDataTile(key, TerrainLayerType::Normalmap, res) {}
    CPUTileSlot<glm::vec4>* cpuData{nullptr};
};

class CPUNormalDataTileProducer : public DataTileProducer, public DataTileSampler<CPUNormalsDataTile> {
private:
    using NormalmapCPUTileBuffer = CPUTileBuffer<glm::vec4>;
    using NormalmapCPUTileCache  = CPUTileCache<TerrainTileKey, glm::vec4>;
    using NormalmapCPUTileSlot   = NormalmapCPUTileBuffer::Slot;

private:
    CPUElevationDataTileProducer* _residualProducer{nullptr};

    std::map<TerrainTileKey, CPUNormalsDataTile*> _dataTiles;

    std::unique_ptr<NormalmapCPUTileBuffer>                      _cpuTileBuffer;
    std::unique_ptr<NormalmapCPUTileCache>                       _cpuTileCache;
    std::unique_ptr<BlockingQueue<GenerateNormalmapTaskResults>> _generateNormalmapTaskOutput;
    std::unordered_map<TerrainTileKey, TaskPtr> _pendingTasks;

public:
    CPUNormalDataTileProducer(CPUElevationDataTileProducer* residualProducer)
        : DataTileProducer(TerrainLayerType::Normalmap, residualProducer->tileResolution)
        , _residualProducer(residualProducer) {
        _cpuTileBuffer.reset(new NormalmapCPUTileBuffer(DataTileProducer::tileResolution.x, DataTileProducer::tileResolution.y, 128));
        _cpuTileCache.reset(new NormalmapCPUTileCache(_cpuTileBuffer.get(), [&](const TerrainTileKey& key, const NormalmapCPUTileSlot* slot) {
            auto it = _dataTiles.find(key);
            if (it != end(_dataTiles)) {
                CPUNormalsDataTile* tile = it->second;
                tile->cpuData            = nullptr;
                _dataTiles.erase(it);
                delete tile;
            }
        }));

        _generateNormalmapTaskOutput.reset(new BlockingQueue<GenerateNormalmapTaskResults>());
    }
    ~CPUNormalDataTileProducer() {}

    // DataTileProducer interface
    virtual CPUNormalsDataTile* GetTile(const TerrainQuadNode& node) final {
        auto it = _dataTiles.find(node.key);
        if (it == _dataTiles.end()) {
            return nullptr;
        } else {
            // touching cached data
            _cpuTileCache->find(node.key);
            return it->second;
        }
    }

    virtual void Update(const std::vector<const TerrainQuadNode*>& nodesInScene, const std::set<TerrainTileKey>& keysLeaving,
                        const std::set<TerrainTileKey>& keysEntering) final {

        // process arrivals
        std::vector<GenerateNormalmapTaskResults> completed;
        _generateNormalmapTaskOutput->flush(&completed);

        for (const GenerateNormalmapTaskResults& results : completed) {
            if (_pendingTasks.find(results.key) == end(_pendingTasks)) {
                EDPLog_W("Processing arrival of unexpected key %s -- skipping", toString(results.key).c_str());
                continue;
            }
            dg_assert_nm(_dataTiles.find(results.key) == end(_dataTiles));

            CPUNormalsDataTile* normalsDataTile = new CPUNormalsDataTile(results.key, tileResolution);

            bool wasCPUTileInCache = _cpuTileCache->get(results.key, &normalsDataTile->cpuData);
            dg_assert_nm(!wasCPUTileInCache);
            dg_assert_nm(normalsDataTile->cpuData);

            normalsDataTile->cpuData->data = results.data;
            _dataTiles.insert({results.key, normalsDataTile});
            _pendingTasks.erase(results.key);
        }

        // cancel tasks
        for (const TerrainTileKey& key : keysLeaving) {
            auto it = _pendingTasks.find(key);
            if (it != end(_pendingTasks)) {
                it->second->tryCancel();
            }
            _pendingTasks.erase(key);
        }

        // push tasks
        std::vector<TaskPtr> tasksToQueue;

        for (const TerrainQuadNode* node : nodesInScene) {
            TerrainDataTile* tile = GetTile(*node);
            if (tile) {
                continue;
            }

            if (_pendingTasks.find(node->key) != end(_pendingTasks)) {
                continue;
            }

            NormalmapCPUTileSlot* cpuSlot = _cpuTileCache->find(node->key);

            if (cpuSlot) {
                CPUNormalsDataTile* normalsDataTile = new CPUNormalsDataTile(node->key, tileResolution);
                normalsDataTile->cpuData            = cpuSlot;
                _dataTiles.insert({node->key, normalsDataTile});
                continue;
            }

            // not cached, need to generate
            CPUElevationDataTile* cpuElevationData = reinterpret_cast<CPUElevationDataTile*>(_residualProducer->GetTile(*node));
            if (cpuElevationData == nullptr || cpuElevationData->cpuData == nullptr) {
                // wait still waiting for elevation data to be generated
                // TODO:: need to prod producer to generate the elevation data
                continue;
            } else {
                TaskPtr task = std::make_shared<GenerateNormalmapTask>(node->key, cpuElevationData, _generateNormalmapTaskOutput.get());
                _pendingTasks.emplace(node->key, task);
                tasksToQueue.push_back(task);
            }
        }

        if (tasksToQueue.size() > 0) {
            scheduler()->queue()->enqueueAll(tasksToQueue);
        }
    }

    // DataTileSampler interface
    virtual CPUNormalsDataTile* FindTile(const TerrainTileKey& key) final { return nullptr; };
};
