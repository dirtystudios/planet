#ifndef __gpu_tile_buffer_h__
#define __gpu_tile_buffer_h__

#include "ResourceTypes.h"
#include "GPUTile.h"
#include <list>
#include "Log.h"

class GPUTextureArrayBuffer {
private:
    uint32_t _bufferSize;
    uint32_t _numTiles;
    gfx::TextureId _textureId;
    GPUTile* _tiles;
    std::list<GPUTile*> _unused;
    std::list<GPUTile*> _used;

public:
    GPUTextureArrayBuffer(uint32_t bufferSize, uint32_t numTiles, gfx::TextureId textureId) {
        _bufferSize = bufferSize;
        _numTiles   = numTiles;
        _textureId  = textureId;
        _tiles = new GPUTile[_numTiles];
        for (uint32_t i = 0; i < _numTiles; ++i) {
            _tiles[i].textureId = _textureId;
            _tiles[i].index = i;
            _unused.push_back(&_tiles[i]);
        }
    }

    ~GPUTextureArrayBuffer() { delete[] _tiles; }

    GPUTile* GetFreeTile() {
        GPUTile* tile = NULL;
        if (_unused.size() > 0) {
            tile = _unused.front();
            _unused.pop_front();
            _used.push_back(tile);
            LOG_D("GPUTextureArrayBuffer: used:%d unused:%d", _used.size(), _unused.size());
        }
        return tile;
    }

    void FreeTile(GPUTile* tile) {
        _used.remove(tile);
        _unused.push_front(tile);
    }

    uint32_t GetMaxCapacity() { return _numTiles; }

    uint32_t GetUnusedCount() { return _unused.size(); }

    gfx::TextureId GetTextureId() { return _textureId; }
};

#endif
