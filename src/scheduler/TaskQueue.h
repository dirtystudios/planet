#pragma once

#include "BlockingQueue.h"
#include "Task.h"

class TaskScheduler;

class TaskQueue {
private:
    BlockingQueue<TaskPtr> _queue;

public:
    void enqueue(TaskPtr task) { _queue.enqueue(task); }

    void enqueueAll(const std::vector<TaskPtr>& tasks) { _queue.enqueueAll(tasks); }

    TaskPtr enqueue(std::function<void(void)> function) {
        TaskPtr task = std::make_shared<LambdaTask>(function);
        enqueue(task);
        return task;
    }

private:
    friend TaskScheduler;

    TaskPtr dequeue() {
        TaskPtr task = nullptr;
        _queue.dequeue(&task);
        return task;
    }
};
