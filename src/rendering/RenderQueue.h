#pragma once

#include <algorithm>
#include <vector>
#include <stdint.h>
#include "DrawTask.h"
#include "Frame.h"

class RenderQueue {
private:
    typedef std::pair<uint64_t, graphics::DrawTask*> DrawTaskPair;

    graphics::Frame* _frame;
    std::vector<DrawTaskPair> _tasks;

public:
    RenderQueue(graphics::Frame* frame) : _frame(frame) {}

    graphics::DrawTask* AppendTask(uint64_t key) {
        graphics::DrawTask* task = _frame->CreateTask();
        _tasks.push_back(std::make_pair(key, task));
        return task;
    }

    void Submit() {
        Sort();
        for (const DrawTaskPair& p : _tasks) {
            _frame->SubmitTask(p.second);
        }
    }

private:
    void Sort() {
        std::sort(begin(_tasks), end(_tasks),
                  [](const DrawTaskPair& p1, const DrawTaskPair& p2) { return p1.first < p2.first; });
    }
};