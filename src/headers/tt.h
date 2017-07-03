#ifndef TT_H
#define TT_H

#include <vector>
#include <list>
#include <cstdint>
#include <mutex>
#include <atomic>
#include "map.h"

template<typename Key, typename Value, uint64_t maxSize, typename Hash = std::hash<Key>>
class Cache {
    static_assert(maxSize > 0, "Size must be greater than 0");

    using CacheEntry = std::pair<size_t, Value>;
    using CacheList = std::list<CacheEntry>;
    
    CacheList internalList;
    CacheMap<size_t, typename CacheList::iterator, maxSize> internalMap;
    Hash hashEngine{};
    std::atomic_size_t currSize{0};

    std::mutex mut;
    
    void overwrite() {
        {
            std::lock_guard<std::mutex> lock(mut);
            internalMap.erase(internalList.back().first);
            internalList.pop_back();
        }
        --currSize;
    }
    
    void add(const CacheEntry& newEntry) {
        if (currSize.load() >= maxSize) {
            overwrite();
        }

        {
            std::lock_guard<std::mutex> lock(mut);
            internalList.push_front(newEntry);
            internalMap.insert(newEntry.first, internalList.begin());
        }
        ++currSize;
    }
    
public:

    void erase(Key& k) {
        {
            std::lock_guard<std::mutex> lock(mut);
            internalList.erase(internalMap[hashEngine(k)]);
        }
        internalMap.erase(hashEngine(k));
    }
    
    void clear() {
        {
            std::lock_guard<std::mutex> lock(mut);
            internalList.clear();
        }
        internalMap.clear();
        currSize = 0;
    }
    
    void add(Key& k, const Value& v) {
        add(std::make_pair(hashEngine(k), v));
    }
    
    Value& operator[](Key& k) {
        const auto elem = internalMap[hashEngine(k)];
        {
            std::lock_guard<std::mutex> lock(mut);
            assert(elem != internalList.end());
            if (elem != internalList.begin() && elem != internalList.end()) {
                internalList.splice(internalList.begin(), internalList, elem);
            }
        }
        return elem->second;
    }
    
    bool retrieve(Key& k) {
        return internalMap.find(hashEngine(k));
    }
    
    size_t size() const {
        return currSize.load();
    }
};

#endif
