#ifndef TT_H
#define TT_H

#include <unordered_map>
#include <list>
#include <iterator>
#include <utility>

template<typename Key, typename Value, size_t maxSize>
class Cache {
    typedef std::pair<Key, Value> CacheEntry;
    typedef std::list<CacheEntry> CacheList;
    typedef std::unordered_map<Key, typename CacheList::iterator> CacheMap;
    
    CacheList internalList;
    CacheMap internalMap;
    size_t currSize = 0;
    
    void overwrite() {
        internalMap.erase((*std::prev(internalList.end())).first);
        internalList.pop_back();
        currSize--;
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
    
    void add(const CacheEntry& newEntry) {
        if (currSize >= maxSize) {
            overwrite();
        }
        internalList.push_front(newEntry);
        internalMap[newEntry.first] = internalList.begin();
        currSize++;
    }
    
    void add(const Key& k, const Value& v) {
        add(std::make_pair(k, v));
    }
    
    Value& operator[](const Key& k) {
        return internalMap[k]->second;
    }
    
    bool retrieve(const Key& k) const {
        return internalMap.find(k) != internalMap.end();
    }
    
    size_t size() const {
        return currSize;
    }
    
};

#endif
