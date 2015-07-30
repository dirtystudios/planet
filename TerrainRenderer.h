#ifndef __terrain_renderer_h__
#define __terrain_renderer_h__

#include "GLHelpers.h"
#include <vector>
#include <list>
#include <unordered_map>
#include "BoundingBox.h"
#include <queue>
#include "GPUTileBuffer.h"


struct TerrainNode {
    uint32_t lod;
    uint32_t tx;
    uint32_t ty;

    double x;
    double y;

    BoundingBox bbox;
    float size;
    GLuint vao, vb;
    TerrainNode* children = { NULL };
};

struct Terrain {
    uint32_t max_lod;

    // Note(eugene): hierarchical transforms
    //Transform transform;
    uint32_t size;
    uint32_t resolution;
    float split_factor;

    //TerrainGenerator terrain_generator;
    TerrainNode* root { NULL };
};

struct Tile {
    Tile(uint32_t lod, uint32_t tx, uint32_t ty, GPUTile* data) : lod(lod), tx(tx), ty(ty), data(data) {};
    
    uint32_t lod;
    uint32_t tx;
    uint32_t ty;

    GPUTile* data;
};


class LRUTileCache {
private:
    typedef size_t TileId;
    typedef std::unordered_map<TileId, Tile*>::iterator CacheIterator;

    GPUTileBuffer* gpu_tile_buffer;
    std::list<TileId> lru;
    std::unordered_map<TileId, Tile*> cache;        

private:
     TileId GetTileId(uint32_t lod, uint32_t tx, uint32_t ty) {
        size_t seed = 0;
        HashCombine(seed, lod);
        HashCombine(seed, tx);
        HashCombine(seed, ty);
        return seed;
    } 
public:
    LRUTileCache(GPUTileBuffer* gpu_tile_buffer) : gpu_tile_buffer(gpu_tile_buffer) {}
    ~LRUTileCache() {
        for(std::pair<TileId, Tile*> p : cache) {
            delete p.second;
        }        
    }
    
    Tile* Get(uint32_t lod, uint32_t tx, uint32_t ty, std::function<void(GPUTile* gpu_tile)> load_func) {
        TileId key = GetTileId(lod, tx, ty);
        CacheIterator it = cache.find(key);

        Tile* val = NULL;        
        if(it != cache.end()) {            
            lru.remove(key);
            lru.push_front(key);
            val = it->second;
        } else {            
            GPUTile* gpu_tile = gpu_tile_buffer->GetFreeTile();
            if(gpu_tile) {
                load_func(gpu_tile);                
                val = new Tile(lod, tx, ty, gpu_tile);
                lru.push_front(key);
                cache.insert(std::make_pair(key, val));                                
            } else {           
                TileId key_to_evict = lru.back();
                it = cache.find(key_to_evict);
                if(it != cache.end()) {                    
                    lru.pop_back();                    

                    val = it->second;
                    //LOG_D("Evicting %d %d %d for %d %d %d", val->lod, val->tx, val->ty, lod, tx, ty); 
                    val->lod = lod;
                    val->tx = tx;
                    val->ty = ty;
                    load_func(val->data);
                    
                    cache.erase(it);         
                    lru.push_front(key);
                    cache.insert(std::make_pair(key, val));                    
                } else {
                    // boom
                    LOG_E("%s", "Failed to evict from LRUTileCache");
                    assert(false);
                }
            }
        }
        return val;
    }        
};

#endif