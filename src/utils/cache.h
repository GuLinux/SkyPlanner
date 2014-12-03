/*
 * Copyright (C) 2014  Marco Gulino <marco.gulino@gmail.com>
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */
#ifndef CACHE_H
#define CACHE_H
#include <boost/date_time.hpp>
#include <list>


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
      std::list<KeyType> expired_keys;
      for(auto element: cache)
	if(!element.second.valid(validity))
	  expired_keys.push_back(element.first);
      for(auto expired: expired_keys) {
	std::cerr << __PRETTY_FUNCTION__ << " Removing expired element: " << expired << std::endl;
	cache.erase(expired);
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
