#ifndef TT_H
#define TT_H

#include <unordered_map>
#include <vector>
#include <list>
#include <array>
#include <iterator>
#include <utility>
#include "map.h"

template<typename Key, typename Value, size_t maxSize, typename Hash = std::hash<Key>>
class Cache {
    static_assert(maxSize > 0, "Size must be greater than 0");

    typedef std::pair<size_t, Value> CacheEntry;
    typedef std::list<CacheEntry> CacheList;
    
    CacheList internalList;
    CacheMap<size_t, typename CacheList::iterator, maxSize> internalMap;
    Hash hashEngine{};
    size_t currSize = 0;
    
    void overwrite() {
        internalMap.erase(internalList.back().first);
        internalList.pop_back();
        currSize--;
    }
    
    void add(const CacheEntry& newEntry) {
        if (currSize >= maxSize) {
            overwrite();
        }
        internalList.push_front(newEntry);
        internalMap[newEntry.first] = internalList.begin();
        currSize++;
    }
    
public:
    void erase(Key& k) {
        internalList.erase(internalMap[hashEngine(k)]);
        internalMap.erase(hashEngine(k));
    }
    
    void clear() {
        internalList.clear();
        internalMap.clear();
        currSize = 0;
    }
    
    void add(Key& k, const Value& v) {
        add(std::make_pair(hashEngine(k), v));
    }
    
    Value& operator[](Key& k) {
        const auto& elem = internalMap[hashEngine(k)];
        if (elem != internalList.begin()) {
            internalList.splice(internalList.begin(), internalList, elem, std::next(elem));
        }
        return elem->second;
    }
    
    bool retrieve(Key& k) {
        return internalMap.find(hashEngine(k));
    }
    
    size_t size() const {
        return currSize;
    }
};

#endif
