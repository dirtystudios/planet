#pragma once

#include <array>
#include <list>

template <class T, size_t N>
class Pool {
private:
    std::array<T, N> _items;
    size_t _poolIdx{0};
    std::list<T*> _freeList;

public:
    T* Get() {
        T* ret = nullptr;
        if (!_freeList.empty()) {
            ret = _freeList.front();
            _freeList.pop_front();
        } else if (_poolIdx < N) {
            ret = &_items[_poolIdx++];
        }
        return ret;
    }

    void Release(T* v) { _freeList.push_front(v); }
};
