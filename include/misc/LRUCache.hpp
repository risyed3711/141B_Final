//
//  LRUCache.hpp
//
//  Created by rick gessner on 5/27/23.
//

#ifndef LRUCache_h
#define LRUCache_h

#include <unordered_map>
#include <list>
#include <cstddef>
#include <stdexcept>

namespace ECE141 {

  template<typename KeyT, typename ValueT>
  class LRUCache {
  public:
    LRUCache(size_t aCapacity=200);
        
    size_t size() const;
    
    bool contains(const KeyT& key) const;

    void evictIf();
    
    //only call this if you know the key is new
    void add(const KeyT& key, const ValueT& value);
    void put(const KeyT& key, const ValueT& value);
    
    const ValueT& get(const KeyT& key);
    
    void clear();
    bool erase(const KeyT &aKey);
    bool remove(const KeyT& aKey);

  };
}

#endif /* LRUCache_h */
