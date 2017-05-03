#pragma once

#include <atomic>
#include <functional>
#include <memory>

enum class TaskState : uint8_t {
    Pending,
    Running,
    Canceling, // attempting to cancelstill running
    Completed,
    Canceled
};

class Task {
private:
    std::atomic<TaskState> _state{TaskState::Pending};

public:
    void run() {
        if (!isCanceled()) {
            _state = TaskState::Running;
            execute();
        }

        _state = _state == TaskState::Canceling ? TaskState::Canceled : TaskState::Completed;
    }

    void tryCancel() { _state = TaskState::Canceling; };

    TaskState state() const { return _state; }
    bool      isRunning() const { return _state == TaskState::Running; }
    bool      isCanceled() const { return _state == TaskState::Canceling; }

protected:
    virtual void execute() = 0;
};

class LambdaTask : public Task {
private:
    std::function<void(void)> _lambda;

public:
    LambdaTask(std::function<void(void)> lambda)
        : _lambda(lambda) {}

protected:
    virtual void execute() final { _lambda(); }
};

using TaskPtr = std::shared_ptr<Task>;
