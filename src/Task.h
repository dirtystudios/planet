#pragma once

#include <functional>
#include <memory>

class Task {
public:
    virtual void execute(){};
};

class LambdaTask : public Task {
private:
    std::function<void(void)> _lambda;

public:
    LambdaTask(std::function<void(void)> lambda) : _lambda(lambda) {}

    virtual void execute() final { _lambda(); }
};

using TaskPtr = std::shared_ptr<Task>;
