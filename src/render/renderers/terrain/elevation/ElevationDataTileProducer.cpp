#include "ElevationDataTileProducer.h"
#include "ConsoleCommands.h"
#include "File.h"
#include "Image.h"
#include "MeshGeneration.h"
#include "TaskScheduler.h"

ElevationDataTileProducer::ElevationDataTileProducer(gfx::RenderDevice* device, const glm::uvec2& tileResolution)
    : DataTileProducer(TerrainLayerType::Heightmap), _device(device), _tileResolution(tileResolution) {

    config::ConsoleCommands::getInstance().RegisterCommand("dumphm", [&](const std::vector<std::string>& params) -> std::string {
        this->dumpCachedHeightmapsToDisk();
        return "success";
    });

    _gpuTileBuffer.reset(new HeightmapGPUTileBuffer(_device, _tileResolution.x, _tileResolution.y, 256));
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
    _tileGeometry.reset(new MeshGeometry(_device, {geometryData}));
}

ElevationDataTileProducer::~ElevationDataTileProducer() {}

ElevationDataTile* ElevationDataTileProducer::GetTile(const TerrainQuadNode& node) {
    auto it = _dataTiles.find(node.key);
    return it == _dataTiles.end() ? nullptr : it->second;
}

void ElevationDataTileProducer::Update(const std::vector<const TerrainQuadNode*>& nodesInScene) {
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

void ElevationDataTileProducer::GenerateHeightmapRegion(const glm::vec2& regionCenter, const glm::vec2& regionSize, const glm::uvec2& resolution,
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

ElevationDataTile* ElevationDataTileProducer::FindTile(const TerrainTileKey& key) {
    TerrainTileKey k  = key;
    auto           it = _dataTiles.find(k);
    while (k.lod != 0 && it == end(_dataTiles)) {
        k  = getParentKey(k);
        it = _dataTiles.find(k);
    }
    return it == _dataTiles.end() ? nullptr : it->second;
}

void ElevationDataTileProducer::dumpCachedHeightmapsToDisk() {
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
