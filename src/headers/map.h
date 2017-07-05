/**
* This is part of ChessPlusPlus, a C++14 Chess AI
* Copyright (C) 2017 John Agapeyev
* 
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MAP_H
#define MAP_H

#include <cstdint>
#include <mutex>

/**
 * CacheMap is a fixed size thread-safe hashmap implementation that does not store the key.
 * I found that the STL unordered_map stored the key for rehashing purposes,
 * and since my key value was large (~1KiB) it was a waste of memory for something that was fixed size.
 * This container originally used chaining to resolve collisions, though that had 
 * an unacceptable level of memory growth, so it was replaced with this fixed memory scheme.
 */
template<typename Key, typename Value, uint64_t maxSize, typename Hash = std::hash<Key>>
class CacheMap {
    static_assert(maxSize > 0, "Size must be greater than zero");
    std::vector<std::vector<std::pair<Value, size_t>>> internalArray{maxSize};
    Hash hashEngine{};

    std::mutex mut;
    
    size_t getBoundedHash(const Key& k) const {
        return hashEngine(k) % maxSize;
    }
    
public:
    void insert(const std::pair<Key, Value>& p) {
        insert(p.first, p.second);
    }
    
    void insert(const Key& k, const Value& v) {
        std::lock_guard<std::mutex> lock(mut);
        if (internalArray[getBoundedHash(k)].empty()) {
            internalArray[getBoundedHash(k)].emplace_back(v, hashEngine(k));
        } else {
            internalArray[getBoundedHash(k)][0] = std::make_pair(v, hashEngine(k));
        }
    }
    
    Value& operator[](const Key& k) {
        const auto bounded = getBoundedHash(k);
        const auto fullHash = hashEngine(k);
        
        {
            std::lock_guard<std::mutex> lock(mut);

            if (internalArray[bounded].empty()) {
                insert(k, Value());
                return internalArray[bounded].back().first;
            }

            for (auto& p : internalArray[bounded]) {
                if (p.second == fullHash) {
                    return p.first;
                }
            }
        }
        
        /*
         * In the event element wasn't inserted due to empty chain, and
         * an existing element wasn't found, insert a default element and return that
         */
        insert(k, Value());

        std::lock_guard<std::mutex> lock(mut);
        return internalArray[bounded].back().first;
    }
    
    bool find(const Key& k) {
        const auto bounded = getBoundedHash(k);
        const auto fullHash = hashEngine(k);
        
        std::lock_guard<std::mutex> lock(mut);
        for (const auto& p : internalArray[bounded]) {
            if (p.second == fullHash) {
                return true;
            }
        }
        return false;
    }
    
    void erase(const Key& k) {
        //Erase vector by replacing it with its default state
        std::lock_guard<std::mutex> lock(mut);
        internalArray[getBoundedHash(k)].clear();
    }
    
    void clear() {
        //Clears the entire array by resetting it back to default values
        std::lock_guard<std::mutex> lock(mut);
        for (const auto& elem : internalArray) {
            elem.clear();
        }
    }
};

#endif
