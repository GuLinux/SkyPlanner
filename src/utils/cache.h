#ifndef CACHE_H
#define CACHE_H
#include <boost/date_time.hpp>


template<typename Payload, typename KeyType>
class Cache {
public:
    Cache(const boost::posix_time::time_duration &validity) : validity(validity) {}
    void put(const KeyType &key, const Payload &payload) {
        cache[key] = {payload, boost::posix_time::second_clock().local_time() };
    }

    Payload value(const KeyType &key) {
        cleanup();
        if(cache.count(key) == 0) {
            return {};
        }
        return cache.at(key).payload;
    }

    void cleanup() {
        for(auto entry: cache) {
            if(!entry.second.valid(validity))
                cache.erase(entry.first);
        }
    }

private:
    struct CacheEntry {
        Payload payload;
        boost::posix_time::ptime when;
        bool valid(const boost::posix_time::time_duration &validity) const {
            return !when.is_not_a_date_time() && (boost::posix_time::second_clock().local_time() - when) <= validity;
        }
    };

    struct
    std::map<KeyType, CacheEntry> cache;
    boost::posix_time::time_duration validity;
};


#endif // CACHE_H
