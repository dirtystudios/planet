#pragma once

#include "RenderDevice.h"
#include "TileBuffer.h"

template <gfx::PixelFormat F>
struct GPUTileSlot : public TileBufferSlot {
    gfx::TextureId texture{0};
    int32_t        slotIndex{-1};
};

template <gfx::PixelFormat F>
class GPUTileBuffer : public TileBuffer<GPUTileSlot<F>> {
public:
    using Slot = GPUTileSlot<F>;

private:
    gfx::RenderDevice* _device{nullptr};
    gfx::TextureId     _textureArray{0};
    uint32_t           _textureArrayIdx{0};

public:
    GPUTileBuffer(gfx::RenderDevice* device, uint32_t width, uint32_t height, uint32_t capacity) : TileBuffer<Slot>(width, height, capacity), _device(device) {
        _textureArray = _device->CreateTextureArray(F, 1, width, height, capacity);
    }
    ~GPUTileBuffer() {}

    virtual void onGetSlot(Slot* slot) final {
        if (slot != nullptr && slot->texture == 0) {
            dg_assert(_textureArrayIdx < TileBuffer<Slot>::_slotPool->capacity(), "%d < %d", _textureArrayIdx, TileBuffer<Slot>::_slotPool->capacity());
            slot->texture   = _textureArray;
            slot->slotIndex = _textureArrayIdx++;
        }
    }
};
