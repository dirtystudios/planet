#pragma once

#include <atomic>
#include <thread>
#include "Task.h"
#include "TaskQueue.h"

class TaskScheduler {
private:
    std::unique_ptr<TaskQueue> _queue;
    std::thread                _worker;
    std::atomic<bool>          _interruptWorkers{false};

private:
    void workerThreadFunc() {
        while (!_interruptWorkers) {
            TaskPtr task;
            if (_queue->dequeue(&task) == false)
                continue;
            dg_assert_nm(task != nullptr);

            task->run();
        }
    }

public:
    TaskScheduler() {
        _queue.reset(new TaskQueue());
        _worker = std::thread(std::bind(&TaskScheduler::workerThreadFunc, this));
    }

    ~TaskScheduler() {
        _interruptWorkers = true;
        _queue.reset();
        _worker.join();
    }

    TaskQueue* queue() { return _queue.get(); }
};

static TaskScheduler* scheduler() {
    static std::unique_ptr<TaskScheduler> scheduler;
    if (scheduler == nullptr)
        scheduler.reset(new TaskScheduler());
    return scheduler.get();
}
