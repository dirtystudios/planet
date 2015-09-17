#ifndef __gpu_tile_buffer_h__
#define __gpu_tile_buffer_h__

#include "gfx/RenderDevice.h"

struct GPUTile {
    GPUTile() {};
    GPUTile(graphics::TextureHandle tex_id, uint32_t idx) : texture_array_id(tex_id), index(idx) {};

    graphics::TextureHandle texture_array_id;
    uint32_t index;

    void CopyData(uint32_t width, uint32_t height, GLenum format, void* data) {
        gl::UpdateTexture2DArray(texture_array_id, format, GL_FLOAT, width, height, index, data);
    }
};

class GPUTileBuffer {
private:
    uint32_t _tile_size;
    uint32_t _num_tiles;
    graphics::TextureHandle _tex_id;    
    GPUTile* _tiles;
    std::list<GPUTile*> _unused;
    std::list<GPUTile*> _used;
public:
    GPUTileBuffer(uint32_t tile_size, uint32_t num_tiles, graphics::TextureHandle texture_handle) {
        this->_tile_size = tile_size;
        this->_num_tiles = num_tiles;
        _tex_id = texture_handle;
        _tiles = new GPUTile[_num_tiles];
        for(uint32_t i = 0; i < _num_tiles; ++i) {
            _tiles[i].texture_array_id = _tex_id;
            _tiles[i].index = i;
            _unused.push_back(&_tiles[i]);
        }
        
    }

    ~GPUTileBuffer() {        
        delete [] _tiles;
    }

    GPUTile* GetFreeTile() {    
        GPUTile* tile = NULL;
        if(_unused.size() > 0) {
            tile = _unused.front();
            _unused.pop_front();
            _used.push_back(tile);
            LOG_D("GPUTileBuffer: used:%d unused:%d", _used.size(), _unused.size()); 
        }
        return tile; 
    }

    void FreeTile(GPUTile* tile) {
        _used.remove(tile);
        _unused.push_front(tile);
    }

    uint32_t GetMaxCapacity() {
        return _num_tiles;
    }

    uint32_t Get_UnusedCount() {
        return _unused.size();
    }

    GLuint GetTextureId() {
        return _tex_id;
    }
};

#endif