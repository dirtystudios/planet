#pragma once

#include "CPUTileBuffer.h"
#include "DGAssert.h"
#include "GPUTileBuffer.h"
#include "LRUCache.h"
#include "Log.h"

static const std::string kGPUTileCachChannel = "GPUTileCache";
#define GPUTCLog(fmt, ...) LOG(Log::Level::Debug, kGPUTileCachChannel, fmt, ##__VA_ARGS__)

template <typename K, typename V>
class TileCache : private IdObj {
public:
    using Cache              = LRUCache<K, V*>;
    using CacheEntry         = typename Cache::CacheEntry;
    using CacheEntryIterator = typename Cache::CacheEntryIterator;
    using EvictionDelegate   = typename Cache::EvictionDelegate;
    using GetResult          = std::pair<bool, V*>;

protected:
    std::unique_ptr<Cache> _cache;
    TileBuffer<V>*         _tileBuffer;
    EvictionDelegate       _evictionDelegate;

public:
    TileCache(TileBuffer<V>* tileBuffer, EvictionDelegate evictionDelegate = EvictionDelegate()) : _tileBuffer(tileBuffer), _evictionDelegate(evictionDelegate) {
        auto delegate = [&](const K& key, V* val) {
            _evictionDelegate(key, val);
            _tileBuffer->releaseSlot(val);
        };
        _cache.reset(new Cache(tileBuffer->capacity(), delegate));
    }

    // Get a Tile from the cache, whether or not it is in the cache a not.
    // returns {bool, V*}. The bool is true if the value was found in the catch, false if the tile was not matching the key
    bool get(const K& key, V** slot) {
        *slot = find(key);

        bool matchesKey = true;
        if (*slot == nullptr) {
            if (!_tileBuffer->hasCapacity()) {
                GPUTCLog("CacheId:%d GetSlot(%s) - cache miss and no available. Evicting", _id, toString(key).c_str());
                dg_assert_nm(_cache->forceEvict());
                // TODO: handle tiles that reference the slot that just got evicted. They think that have data in the slot
            }

            *slot = _tileBuffer->getFreeSlot();
            dg_assert_nm(*slot != nullptr);
            _cache->put(key, *slot);
            matchesKey = false;
        }

        return matchesKey;
    }

    V* find(const K& key) {
        V* slot = nullptr;
        _cache->get(key, &slot);
        return slot;
    }

    bool evict(const K& key) { return _cache->evict(key); }

    CacheEntryIterator begin() { return _cache->begin(); };
    CacheEntryIterator end() { return _cache->end(); }
    void copyContents(std::vector<CacheEntry>& outputVector) { _cache->copyContents(outputVector); }
};

template <typename K, typename T>
using CPUTileCache = TileCache<K, typename CPUTileBuffer<T>::Slot>;

template <typename K, gfx::PixelFormat F>
using GPUTileCache = TileCache<K, typename GPUTileBuffer<F>::Slot>;
