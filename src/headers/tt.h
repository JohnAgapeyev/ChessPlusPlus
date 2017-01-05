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

    typedef std::pair<Key, Value> CacheEntry;
    typedef std::list<CacheEntry> CacheList;
    
    CacheList internalList;
    CacheMap<Key, typename CacheList::iterator, maxSize, Hash> internalMap;
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
    void erase(const Key& k) {
        internalList.erase(internalMap[k]);
        internalMap.erase(k);
    }
    
    void clear() {
        internalList.clear();
        internalMap.clear();
        currSize = 0;
    }
    
    void add(const Key& k, const Value& v) {
        add(std::make_pair(k, v));
    }
    
    Value& operator[](const Key& k) {
        const auto& elem = internalMap[k];
        internalList.splice(internalList.begin(), internalList, elem, std::next(elem));
        return elem->second;
    }
    
    bool retrieve(const Key& k) {
        return internalMap.find(k);
    }
    
    size_t size() const {
        return currSize;
    }
};

#endif
