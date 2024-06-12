#ifndef Cache_hpp
#define Cache_hpp

#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include <list>
#include <algorithm>
#include <deque>

template<typename KeyT, typename ValueT>
class LRUCache {
public:

    //OCF
    LRUCache(size_t aSize=10) : maxsize(aSize) {}
    LRUCache(const LRUCache& aCache) {maxsize=aCache.maxsize;data=aCache.data;keys=aCache.keys;}
    ~LRUCache() = default;

    void update(const KeyT &key, const ValueT& value){
        auto it = data.find(key);
        keys.splice(keys.begin(),keys,it->second.second);
//        auto old = it->second.second;
//        keys.push_front(it->first);
//        it->second.second=keys.begin();
//        keys.erase(old);
        it->second.first=value;
    }
    void put(const KeyT &key, const ValueT& value){
        if(size()==maxsize)
        {auto lastKey=*keys.rbegin();keys.pop_back();data.erase(lastKey);}
        keys.push_front(key);
        data[key]=std::make_pair(value,keys.begin());
        //push everything back add to front of queue
    }
    ValueT& get(const KeyT& key){
        auto it = data.find(key);
        keys.splice(keys.begin(),keys,it->second.second);
//        auto old = it->second.second;
//        keys.push_front(it->first);
//        it->second.second=keys.begin();
//        keys.erase(old);
        return it->second.first;
            //move to front of cache, move all others to back
            //return value
    }
    bool contains(const KeyT& key) const{
        return data.find(key)!=data.end();
    }
    size_t  size() const{
        return data.size();
    }

protected:
    size_t maxsize; //prevent cache from growing past this size...
    std::list<KeyT> keys;
    std::unordered_map<KeyT,std::pair<ValueT,typename std::list<KeyT>::iterator>> data;

    //data members here...
};

#endif //Cache_hpp