#include "ElevationDataTileProducer.h"
#include "ConsoleCommands.h"
#include "File.h"
#include "Image.h"
#include "MeshGeneration.h"
#include "TaskScheduler.h"

static const std::string kEDPChannel = "tileproducer.elevation";
#define EDPLog_W(fmt, ...) LOG(Log::Level::Warn, kEDPChannel, fmt, ##__VA_ARGS__)

ElevationDataTileProducer::ElevationDataTileProducer(gfx::RenderDevice* device, CPUElevationDataTileProducer* cpuElevationDataTileProducer)
    : DataTileProducer(TerrainLayerType::Heightmap, cpuElevationDataTileProducer->tileResolution)
    , _device(device)
    , _dataProducer(cpuElevationDataTileProducer) {

    _gpuTileBuffer.reset(new HeightmapGPUTileBuffer(_device, DataTileProducer::tileResolution.x, DataTileProducer::tileResolution.y, 128));
    _gpuTileCache.reset(new HeightmapGPUTileCache(_gpuTileBuffer.get(), [&](const TerrainTileKey& key, const HeightmapGPUTileSlot* slot) {
        auto it = _dataTiles.find(key);
        if (it != end(_dataTiles)) {
            GPUElevationDataTile* tile = it->second;
            _dataTiles.erase(key);
            delete tile;
        }

    }));

    MeshGeometryData geometryData;
    dgen::GenerateGrid(glm::vec3(0, 0, 0), {2, 2}, DataTileProducer::tileResolution, &geometryData);
    _tileGeometry.reset(new MeshGeometry(_device, {geometryData}));
}

ElevationDataTileProducer::~ElevationDataTileProducer() {}

GPUElevationDataTile* ElevationDataTileProducer::GetTile(const TerrainQuadNode& node) {
    auto it = _dataTiles.find(node.key);
    if (it == _dataTiles.end()) {
        return nullptr;
    } else {
        // touching cached data
        _gpuTileCache->find(node.key);
        return it->second;
    }
}

void ElevationDataTileProducer::Update(const std::vector<const TerrainQuadNode*>& nodesInScene, const std::set<TerrainTileKey>& keysLeaving,
                                       const std::set<TerrainTileKey>& keysEntering) {
    for (const TerrainQuadNode* node : nodesInScene) {
        TerrainDataTile* tile = GetTile(*node);
        if (tile == nullptr) {
            HeightmapGPUTileSlot* gpuSlot = _gpuTileCache->find(node->key);

            if (gpuSlot == nullptr) {
                // not cached, need cpu data

                CPUElevationDataTile* cpuElevationData = reinterpret_cast<CPUElevationDataTile*>(_dataProducer->GetTile(*node));
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
            GPUElevationDataTile* elevationDataTile = new GPUElevationDataTile(node->key, tileResolution);
            elevationDataTile->gpuData              = gpuSlot;
            elevationDataTile->geometry             = _tileGeometry.get();

            _dataTiles.insert({node->key, elevationDataTile});
        }
    }
}

GPUElevationDataTile* ElevationDataTileProducer::FindTile(const TerrainTileKey& key) {
    TerrainTileKey k  = key;
    auto           it = _dataTiles.find(k);
    while (k.lod != 0 && it == end(_dataTiles)) {
        k  = getParentKey(k);
        it = _dataTiles.find(k);
    }
    return it == _dataTiles.end() ? nullptr : it->second;
}
