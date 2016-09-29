#pragma once

#include <list>
#include <unordered_map>

template <typename K, typename V>
class LRUCache {
public:
    using EvictionDelegate = std::function<void(const K&, V&)>;
    using LoadingDelegate  = std::function<bool(K, V*)>;

private:
    struct CacheEntry {
        K first;
        V second;
    };
    using CacheList = std::list<CacheEntry>;
    using CacheMap  = std::unordered_map<K, typename CacheList::iterator>;

    uint32_t         _maxSize;
    CacheMap         _cacheMap;
    CacheList        _cacheList;
    EvictionDelegate _evictionDelegate;

public:
    LRUCache(uint32_t maxSize, EvictionDelegate evictionDelegate = EvictionDelegate()) : _maxSize(maxSize), _evictionDelegate(evictionDelegate) {}

    ~LRUCache() {
        // for now
        for (CacheEntry& entry : _cacheList) {
            _evictionDelegate(entry.first, entry.second);
        }
    }

    void put(const K& key, V data) {
        auto it = _cacheMap.find(key);
        if (it != _cacheMap.end()) {
            _cacheMap.erase(it);
            _cacheList.erase(it->second);
        }
        CacheEntry entry;
        entry.first  = key;
        entry.second = data;
        _cacheList.push_front(entry);
        _cacheMap.insert(std::make_pair(key, _cacheList.begin()));

        tryEvict();
    }

    bool get(const K& key, V* outVal) {
        auto it = _cacheMap.find(key);
        if (it == _cacheMap.end()) {
            return false;
        }

        _cacheList.splice(_cacheList.begin(), _cacheList, it->second);
        *outVal = it->second->second;
        return true;
    }

    bool get(const K& key, V* outVal, LoadingDelegate loadingDelegate) {
        if (!get(key, outVal)) {
            if (loadingDelegate(key, outVal) && *outVal != nullptr) {
                put(key, *outVal);
                return true;
            } else {
                return false;
            }
        }
        return true;
    }

    bool evict(const K& key) {
        auto it = _cacheMap.find(key);
        if (it == end(_cacheMap))
            return false;

        _evictionDelegate(it->second->first, it->second->second);
        _cacheList.erase(it->second);
        _cacheMap.erase(it);
        return true;
    }

    bool tryEvict() {
        if (_cacheMap.size() > _maxSize) {
            auto iterToEvict = _cacheList.back();
            _evictionDelegate(iterToEvict.first, iterToEvict.second);
            _cacheList.pop_back();
            _cacheMap.erase(iterToEvict.first);
            return true;
        }
        return false;
    }

    bool forceEvict() {
        if (_cacheMap.size() == 0)
            return false;

        auto iterToEvict = _cacheList.back();
        _evictionDelegate(iterToEvict.first, iterToEvict.second);
        _cacheList.pop_back();
        _cacheMap.erase(iterToEvict.first);
        return true;
    }

    uint32_t size() const { return _cacheMap.size(); }
};
