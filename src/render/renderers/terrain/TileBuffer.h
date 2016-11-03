#pragma once

#include <memory>
#include "IdObj.h"
#include "Pool.h"

struct TileBufferSlot {
    uint64_t tileBufferId{0};
};

template <typename T>
class TileBuffer : protected IdObj {
private:
    static_assert(std::is_base_of<TileBufferSlot, T>::value, "T must be a descendant of TileBufferSlot");

protected:
    uint32_t                 _tileWidth;
    uint32_t                 _tileHeight;
    std::unique_ptr<Pool<T>> _slotPool;

public:
    TileBuffer(uint32_t tileWidth, uint32_t tileHeight, uint32_t capacity) : _tileWidth(tileWidth), _tileHeight(tileHeight), _slotPool(new Pool<T>(capacity)){};
    virtual ~TileBuffer() {}

    T* getFreeSlot() {
        T* slot = _slotPool->construct();
        dg_assert_nm(slot != nullptr);
        dynamic_cast<TileBufferSlot*>(slot)->tileBufferId = _id;
        onGetSlot(slot);
        return slot;
    }
    void releaseSlot(T* slot) {
        dg_assert_nm(dynamic_cast<TileBufferSlot*>(slot)->tileBufferId == _id);
        onReleaseSlot(slot);
        return _slotPool->release(slot);
    };
    bool     hasCapacity() const { return _slotPool->freeCount() > 0; }
    uint32_t capacity() const { return _slotPool->capacity(); }

protected:
    virtual void onGetSlot(T* slot){};
    virtual void onReleaseSlot(T* slot){};
};
