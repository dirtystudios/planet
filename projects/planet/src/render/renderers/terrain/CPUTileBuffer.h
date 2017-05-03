#pragma once

#include <vector>
#include "TileBuffer.h"

template <typename T>
struct CPUTileSlot : public TileBufferSlot {
    std::vector<T> data;
};

template <typename T>
class CPUTileBuffer : public TileBuffer<CPUTileSlot<T>> {
public:
    using Slot = CPUTileSlot<T>;

public:
    CPUTileBuffer(uint32_t tileWidth, uint32_t tileHeight, uint32_t capacity) : TileBuffer<Slot>(tileWidth, tileHeight, capacity) {}

    virtual void onGetSlot(Slot* slot) final {
        if (slot != nullptr && slot->data.capacity() == 0) {
            slot->data.reserve(TileBuffer<Slot>::_tileWidth * TileBuffer<Slot>::_tileHeight);
        }
    }
};
