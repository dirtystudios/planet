#pragma once

#include <deque>
#include <mutex>
#include <thread>
using namespace std;

template <class T>
class BlockingQueue {
private:
    bool                    _shutdown;
    deque<T>                _data;
    std::mutex              _lock;
    std::condition_variable _cond;

public:
    BlockingQueue() { _shutdown = false; }

    ~BlockingQueue() {}

    void enqueueAll(const std::vector<T>& items) {
        if (items.size() == 0)
            return;
        std::lock_guard<std::mutex> lk(_lock);
        std::move(begin(items), end(items), std::back_inserter(_data));
    }

    void enqueue(T item) {
        std::lock_guard<std::mutex> lk(_lock);
        _data.push_back(item);
        _cond.notify_one();
    }

    void flush(std::vector<T>* dst, int32_t maxItemsToFlush = std::numeric_limits<int32_t>::max()) {
        std::lock_guard<std::mutex> lk(_lock);
        if (_data.size() > 0) {
            maxItemsToFlush = std::min<int32_t>(_data.size(), maxItemsToFlush);
            std::move(begin(_data), begin(_data) + maxItemsToFlush, std::back_inserter(*dst));
            _data.erase(begin(_data), begin(_data) + maxItemsToFlush);
        }
    }

    bool dequeue(T* item) {
        std::unique_lock<std::mutex> lk(_lock);
        while (_data.size() == 0) {
            _cond.wait(lk);
            if (_shutdown) {
                return false;
            }
        }
        *item = _data.front();
        _data.pop_front();
        lk.unlock();
        return true;
    }

    int size() {
        std::lock_guard<std::mutex> lk(_lock);
        int                         size = _data.size();
        return size;
    }

    void shutdown() {
        _shutdown = true;
        _cond.notify_all();
    }
};
