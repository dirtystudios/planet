#include "GenerateHeightmapTask.h"

GenerateHeightmapTask::GenerateHeightmapTask(const TerrainTileKey& key, const dm::Rect3Dd& region, const glm::uvec2& resolution,
                                             BlockingQueue<GenerateHeightmapTaskResults>* outputQueue)
    : _results({key}), _region(region), _resolution(resolution), _outputQueue(outputQueue) {

    _noise.SetSeed(32);
    _noise.SetFrequency(0.05);
    _noise.SetOctaveCount(8);
}

void GenerateHeightmapTask::execute() {
    double dx = _region.width() / (double)(_resolution.x - 1);
    double dy = _region.height() / (double)(_resolution.y - 1);

    for (uint32_t i = 0; i < _resolution.y; ++i) {
        double     t1       = dy * i / _region.height();
        glm::dvec3 rowStart = dm::lerp(_region.bl(), _region.tl(), t1);
        glm::dvec3 rowEnd   = dm::lerp(_region.br(), _region.tr(), t1);

        for (uint32_t j = 0; j < _resolution.x; ++j) {
            double     t2     = dx * j / _region.width();
            glm::dvec3 sample = dm::lerp(rowStart, rowEnd, t2);
            sample *= 0.005f;
            double val = _noise.GetValue(sample.x, sample.y, sample.z);

            if (val > _results.max) {
                _results.max = val;
            } else if (val < _results.min) {
                _results.min = val;
            }

            _results.data.push_back(val);
        }
    }

    if (!isCanceled()) {
        _outputQueue->enqueue(_results);
    }
}
