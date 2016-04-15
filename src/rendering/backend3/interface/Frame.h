#pragma once

#include "DrawTask.h"
#include <string>

namespace graphics {
class Frame {
private:
    std::vector<TextureUpdate*> _textureUpdates;
    std::vector<BufferUpdate*> _bufferUpdates;

    DrawTask _tasks[128];
    uint32_t _idx{0};
    std::vector<DrawTask*> _submittedTasks;

public:
    DrawTask* CreateTask() {
        assert(_idx < 128);
        DrawTask* task = &_tasks[_idx++];
        ClearTask(task);
        return task;
    }

    void SubmitTask(DrawTask* task) { _submittedTasks.push_back(task); }

    void SubmitTask(TextureUpdate* task) { _textureUpdates.push_back(task); }

    void SubmitTask(BufferUpdate* task) { _bufferUpdates.push_back(task); }

    const std::vector<BufferUpdate*>& GetBufferUpdates() { return _bufferUpdates; }

    const std::vector<TextureUpdate*>& GetTextureUpdates() { return _textureUpdates; }

    const std::vector<DrawTask*>& GetDrawTasks() { return _submittedTasks; }

    void Clear() {
        _idx = 0;
        _submittedTasks.clear();
        _textureUpdates.clear();
        _bufferUpdates.clear();
    }

    std::string ToString() {
        return "Frame [textureUpdates:" + std::to_string(_textureUpdates.size()) + ", bufferUpdates: " +
               std::to_string(_bufferUpdates.size()) + ", submittedTasks: " + std::to_string(_submittedTasks.size()) +
               "]";
    }

private:
    void ClearTask(DrawTask* task) {
        for (BufferUpdate* update : task->bufferUpdates) {
            delete update;
        }
        for (TextureBind* bind : task->textureBinds) {
            delete bind;
        }
        for (ShaderParamUpdate* update : task->shaderParamUpdates) {
            delete update;
        }
        for (TextureUpdate* update : task->textureUpdates) {
            delete update;
        }

        task->textureUpdates.clear();
        task->shaderParamUpdates.clear();
        task->textureBinds.clear();
        task->bufferUpdates.clear();

        memset(task, 0, sizeof(DrawTask));
    }
};
}