/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Marco Gulino <email>
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

#ifndef UTILS_H
#define UTILS_H

#include <functional>
#include <ios>
#include <sstream>
#include <algorithm>
#include "types.h"
#include "geocoder.h"

class Utils
{
public:
  static int exponentialPercentage(double value, double limit, double base);
  template<typename T>
  static T fromHexString(const std::string &s) {
    std::stringstream ss;
    ss << std::hex << s;
    T t;
    ss >> t;
    return t;
  }
  static std::string sanitizeForURL(const std::string &in, const std::string &replacement = "-");
  static std::string csv(const std::string &value, char separator=',');
  static std::string mcg_name_fix(const std::string &src);
  template<typename T, typename E> static void copy(const T &source, E outputIterator) { std::copy(std::begin(source), std::end(source), outputIterator); }
  template<typename T, typename E, typename F> static void copy_if(const T &source, E outputIterator, F condition) { std::copy_if(std::begin(source), std::end(source), outputIterator, condition); }
};

class Scope {
public:
  Scope(const std::function<void()> &f) : f(f) {};
  ~Scope() { f(); }
private:
  std::function<void()> f;
};

template<typename Payload, typename KeyType>
class Cache {
public:
    Cache(const boost::posix_time::time_duration &validity) : validity(validity) {}
    void put(const KeyType &key, const Payload &payload) {
        cache[key] = {payload, boost::posix_time::second_clock().local_time() };
    }

    Payload value(const KeyType &key) {
        cleanup();
        if(cache.count(key) == 0)
            return {};
        return cache.at(key).payload;
    }

    void cleanup() {
        for(auto entry: cache) {
            std::cout << "** entry: " << entry.first << ", validity: " << boost::posix_time::to_simple_string(validity)
                         << ", when: " << boost::posix_time::to_simple_string(entry.second.when)
                         << ", diff: " << boost::posix_time::to_simple_string(boost::posix_time::second_clock().local_time() - entry.second.when)
                            << std::endl;
            if(!entry.second.valid(validity))
                cache.erase(entry.first);
        }
    }

private:
    struct CacheEntry {
        Payload payload;
        boost::posix_time::ptime when;
        bool valid(const boost::posix_time::time_duration &validity) const {
            return when.is_not_a_date_time() && boost::posix_time::second_clock().local_time() - when <= validity;
        }
    };


    struct
    std::map<KeyType, CacheEntry> cache;
    boost::posix_time::time_duration validity;
};

#endif // UTILS_H
