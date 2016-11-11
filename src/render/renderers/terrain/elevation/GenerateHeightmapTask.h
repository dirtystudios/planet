#pragma once

#include <glm/glm.hpp>
#include <noise/noise.h>
#include <vector>
#include "BlockingQueue.h"
#include "Rectangle.h"
#include "Task.h"
#include "TerrainTileKey.h"

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
    GenerateHeightmapTask(const TerrainTileKey& key, const dm::Rect3Dd& region, const glm::uvec2& resolution, BlockingQueue<GenerateHeightmapTaskResults>* outputQueue);
    virtual void execute() final;
};
