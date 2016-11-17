#include "CPUElevationDataTileProducer.h"
#include "ConsoleCommands.h"
#include "File.h"
#include "Image.h"
#include "TaskScheduler.h"

static const std::string kEDPChannel = "tileproducer.cpuelevation";
#define EDPLog_W(fmt, ...) LOG(Log::Level::Warn, kEDPChannel, fmt, ##__VA_ARGS__)

CPUElevationDataTileProducer::CPUElevationDataTileProducer(const glm::uvec2& tileResolution)
    : DataTileProducer(TerrainLayerType::Heightmap, tileResolution) {
    config::ConsoleCommands::getInstance().RegisterCommand("dumphm", [&](const std::vector<std::string>& params) -> std::string {
        this->dumpCachedHeightmapsToDisk();
        return "success";
    });
    _cpuTileBuffer.reset(new HeightmapCPUTileBuffer(DataTileProducer::tileResolution.x, DataTileProducer::tileResolution.y, 128));
    _cpuTileCache.reset(new HeightmapCPUTileCache(_cpuTileBuffer.get(), [&](const TerrainTileKey& key, const HeightmapCPUTileSlot* slot) {
        auto it = _dataTiles.find(key);
        if (it != end(_dataTiles)) {
            CPUElevationDataTile* tile = it->second;
            tile->cpuData              = nullptr;
            _dataTiles.erase(it);
            delete tile;
        }
    }));
    _generateHeightmapTaskOutput.reset(new BlockingQueue<GenerateHeightmapTaskResults>());
}

CPUElevationDataTile* CPUElevationDataTileProducer::GetTile(const TerrainQuadNode& node) {
    auto it = _dataTiles.find(node.key);
    if (it == _dataTiles.end()) {
        return nullptr;
    } else {
        // touching cached data
        _cpuTileCache->find(node.key);
        return it->second;
    }
}

void CPUElevationDataTileProducer::Update(const std::vector<const TerrainQuadNode*>& nodesInScene, const std::set<TerrainTileKey>& keysLeaving,
                                          const std::set<TerrainTileKey>& keysEntering) {
    // process arrivals
    std::vector<GenerateHeightmapTaskResults> completed;
    _generateHeightmapTaskOutput->flush(&completed);

    for (const GenerateHeightmapTaskResults& results : completed) {
        if (_pendingTasks.find(results.key) == end(_pendingTasks)) {
            EDPLog_W("Processing arrival of unexpected key %s -- skipping", toString(results.key).c_str());
            continue;
        }
        dg_assert_nm(_dataTiles.find(results.key) == end(_dataTiles));

        CPUElevationDataTile* elevationDataTile = new CPUElevationDataTile(results.key, tileResolution);

        bool wasCPUTileInCache = _cpuTileCache->get(results.key, &elevationDataTile->cpuData);
        dg_assert_nm(!wasCPUTileInCache);
        dg_assert_nm(elevationDataTile->cpuData);

        elevationDataTile->cpuData->data = results.data;
        _dataTiles.insert({results.key, elevationDataTile});
        _pendingTasks.erase(results.key);
    }

    // push tasks
    std::vector<TaskPtr> tasksToQueue;

    // cancel tasks
    for (const TerrainTileKey& key : keysLeaving) {
        auto it = _pendingTasks.find(key);
        if (it != end(_pendingTasks)) {
            it->second->tryCancel();
        }
        _pendingTasks.erase(key);
    }

    for (const TerrainQuadNode* node : nodesInScene) {
        TerrainDataTile* tile = GetTile(*node);
        if (tile == nullptr) {
            if (_pendingTasks.find(node->key) != end(_pendingTasks)) {
                continue;
            }

            HeightmapCPUTileSlot* cpuSlot = _cpuTileCache->find(node->key);
            if (cpuSlot == nullptr) {
                TaskPtr task = std::make_shared<GenerateHeightmapTask>(node->key, node->sampleRect, DataTileProducer::tileResolution, _generateHeightmapTaskOutput.get());
                _pendingTasks.emplace(node->key, task);
                tasksToQueue.push_back(task);
            } else {
                dg_assert_nm(_dataTiles.find(node->key) == _dataTiles.end());

                CPUElevationDataTile* elevationDataTile = new CPUElevationDataTile(node->key, tileResolution);
                elevationDataTile->cpuData              = cpuSlot;
                _dataTiles.insert({node->key, elevationDataTile});
            }
        } else {
            dg_assert_nm(((CPUElevationDataTile*)tile)->cpuData != nullptr);
        }
    }

    if (tasksToQueue.size() > 0) {
        scheduler()->queue()->enqueueAll(tasksToQueue);
    }
}

CPUElevationDataTile* CPUElevationDataTileProducer::FindTile(const TerrainTileKey& key) {
    TerrainTileKey k  = key;
    auto           it = _dataTiles.find(k);
    while (k.lod != 0 && it == end(_dataTiles)) {
        k  = getParentKey(k);
        it = _dataTiles.find(k);
    }
    return it == _dataTiles.end() ? nullptr : it->second;
}

void CPUElevationDataTileProducer::dumpCachedHeightmapsToDisk() {
    // copy cache
    std::vector<HeightmapCPUTileCache::CacheEntry> cacheSnapshot;
    _cpuTileCache->copyContents(cacheSnapshot);

    std::string          rootDir = fs::GetProcessDirectory();
    std::vector<uint8_t> data;
    data.reserve(DataTileProducer::tileResolution.x * DataTileProducer::tileResolution.y);
    for (const HeightmapCPUTileCache::CacheEntry& cacheEntry : cacheSnapshot) {
        data.clear();

        TerrainTileKey        key     = cacheEntry.first;
        HeightmapCPUTileSlot* cpuSlot = cacheEntry.second;
        TerrainTileKey        parent  = getParentKey(key);
        std::string           dirPath = rootDir + "heightmap_dump/terrain_" + std::to_string(key.tid) + "/lod_" + std::to_string(key.lod) + "/" + asFilename(parent);
        std::string           fpath   = dirPath + "/" + asFilename(key) + ".png";
        dg_assert_nm(fs::mkdirs(dirPath));
        std::transform(begin(cpuSlot->data), end(cpuSlot->data), std::back_inserter(data), [&](float d) { return static_cast<uint8_t>((d + 1.0) * 127.5f); });
        dg_assert_nm(dimg::WriteImageToFile(fpath.c_str(), DataTileProducer::tileResolution.x, DataTileProducer::tileResolution.y, dimg::PixelFormat::R8Unorm, data.data()));
    }
}
