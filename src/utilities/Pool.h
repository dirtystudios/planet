#pragma once

#include <list>
#include <vector>

template <class T>
class Pool {
private:
    static_assert(!std::has_virtual_destructor<T>::value, "Unsafe");

    uint32_t       _capacity;
    std::vector<T> _items;
    std::list<T*>  _freeList;

public:
    Pool(uint32_t capacity) : _capacity(capacity) { _items.reserve(_capacity); }

    template <class... Args>
    T* construct(Args&&... args) {
        T* ret = nullptr;
        if (_items.size() < _capacity) {
            _items.emplace_back(std::forward<Args>(args)...);
            ret = &_items.back();
        } else if (!_freeList.empty()) {
            ret = _freeList.front();
            new (ret) T(std::forward<Args>(args)...);
            _freeList.pop_front();
        }
        return ret;
    }

    // not sure if should be calling destructor...well see
    void release(T* v) {
        (*v).~T();
        _freeList.push_front(v);
    }

    uint32_t capacity() const { return _capacity; };
    uint32_t freeCount() const { return _freeList.size() + _capacity - _items.size(); }
};
