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
#include "openweather.h"
#include <Wt/Json/Parser>
#include <Wt/Json/Object>
#include "utils/curl.h"
#include "skyplanner.h"
#include <sstream>
#include <utils/format.h>
#include <Wt/Utils>
#include <boost/date_time.hpp>

using namespace Wt;
using namespace std;
#include <utils/cache.h>
#include <settings.h>

OpenWeather::OpenWeather()
{
}

struct WeatherCacheEntry {
    string key;
    string json;
    operator bool() const { return !json.empty(); }
    static string byName(const string &name, int days, const string &language) {
        return format("by-name-%s-%d-%s") % name % days % language;
    }
    static string byCoordinates(const Coordinates::LatLng &coordinates, int days, const string &language) {
        return format("by-coordinates-%f-%f-%d-%s") % coordinates.latitude.degrees() % coordinates.longitude.degrees() % days % language;
    }
};

shared_ptr<WeatherForecast> OpenWeather::forecast(const Coordinates::LatLng &coordinates, const std::string &cityName, int days)
{
    static auto openweather_api_key = Settings::instance().openweather_api_key();
    if(! openweather_api_key )
      return {};
    days = std::min(days, 16);
    static Cache<WeatherCacheEntry, string> weatherCache(boost::posix_time::hours(6));
    spLog("notice") << "Coordinates: " << coordinates << ", city name: " << cityName << ", days: " << days << ", locale: " << wApp->locale().name();
    string language = wApp->locale().name();
    string cityUrl = format("http://api.openweathermap.org/data/2.5/forecast/daily?q=%s&mode=json&units=metric&cnt=%d&type=like&lang=%s&APPID=%s") % Wt::Utils::urlEncode(cityName) % days % language % *openweather_api_key;
    string coordinatesUrl = format("http://api.openweathermap.org/data/2.5/forecast/daily?lat=%f&lon=%f=json&units=metric&cnt=%d&type=like&lang=%sAPPID=%s") % coordinates.latitude.degrees() % coordinates.longitude.degrees() % days % language % *openweather_api_key;
    string cityCacheKey = WeatherCacheEntry::byName(cityName, days, language);
    string coordinatesCacheKey = WeatherCacheEntry::byCoordinates(coordinates, days, language);
    stringstream out;
    Json::Object weatherForecastObject;
    shared_ptr<WeatherForecast> result;
    auto parseWeather = [&weatherForecastObject, &result](const string &json) {
        Json::ParseError error;
        bool parsed = Json::parse(json, weatherForecastObject, error);
        if(parsed) {
            spLog("notice") << "Parse was ok";
            try {
                result = make_shared<WeatherForecast>(weatherForecastObject);
                return result->cod() == "200";
            } catch(...) {
                spLog("warning") << "Error parsing json: " << json;
                return false;
            }
        }
        return false;
    };

    WeatherCacheEntry cacheEntry = weatherCache.value(cityCacheKey);
    if(!cacheEntry) cacheEntry = weatherCache.value(coordinatesCacheKey);
    if(cacheEntry) {
        spLog("notice") << "Cache hit for " << cityName << ", " << coordinates;
        if(!parseWeather(cacheEntry.json)) {
            spLog("warning") << "Error parsing cache entry: " << cacheEntry.json << ", falling back to service...";
        } else
            return result;
    }
    Curl curl(out);
    spLog("notice") << "Entry not found in cache; asking web service: " << cityUrl;
    if(!cityName.empty() && curl.get(cityUrl).requestOk() && parseWeather(out.str())) {
        spLog("notice") << "Weather ok for city " << cityName;
        weatherCache.put(cityCacheKey, {cityCacheKey, out.str()});
        return result;
    }
    out.str("");
    spLog("notice") << "curl results: " << curl.requestOk() << "-" << curl.httpResponseCode();
    spLog("notice") << "Entry for city didn't match, trying by coordinates: " << coordinatesUrl;
    if(curl.get(coordinatesUrl).requestOk() && parseWeather(out.str())) {
        spLog("notice") << "Weather ok for coordinates " << coordinates;
        weatherCache.put(coordinatesCacheKey, {coordinatesCacheKey, out.str()});
        return result;
    }
    spLog("notice") << "curl results: " << curl.requestOk() << "-" << curl.httpResponseCode();
    return {};
}
