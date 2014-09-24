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
#include <utils/utils.h>

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
    days = std::max(days, 16);
    static Cache<WeatherCacheEntry, string> weatherCache(boost::posix_time::hours(6));
    spLog("notice") << "Coordinates: " << coordinates << ", city name: " << cityName << ", days: " << days << ", locale: " << wApp->locale().name();
    string language = wApp->locale().name();
    string cityUrl = format("http://api.openweathermap.org/data/2.5/forecast/daily?q=%s&mode=json&units=metric&cnt=%d&type=like&lang=%s") % Wt::Utils::urlEncode(cityName) % days % language;
    string coordinatesUrl = format("http://api.openweathermap.org/data/2.5/forecast/daily?lat=%f&lon=%f=json&units=metric&cnt=%d&type=like&lang=%s") % coordinates.latitude.degrees() % coordinates.longitude.degrees() % days % language;
    string cityCacheKey = WeatherCacheEntry::byName(cityName, days, language);
    string coordinatesCacheKey = WeatherCacheEntry::byCoordinates(coordinates, days, language);
    stringstream out;
    Curl curl(out);
    Json::Object weatherForecastObject;
    shared_ptr<WeatherForecast> result;
    auto parseWeather = [&out, &weatherForecastObject, &result] {
        Json::ParseError error;
        bool parsed = Json::parse(out.str(), weatherForecastObject, error);
        if(parsed) {
            spLog("notice") << "Parse was ok";
            try {
                result = make_shared<WeatherForecast>(weatherForecastObject);
                return result->cod() == "200";
            } catch(...) {
                spLog("warning") << "Error parsing json: " << out.str();
                return false;
            }
        }
        return false;
    };

    WeatherCacheEntry cacheEntry = weatherCache.value(cityCacheKey);
    if(!cacheEntry) cacheEntry = weatherCache.value(coordinatesCacheKey);
    if(cacheEntry) {
        out << cacheEntry;
        spLog("notice") << "Weather data found in cache for city " << cityName << ", coordinates " << coordinates;
        parseWeather();
        return result;
    }

    spLog("notice") << "Entry not found in cache; asking web service";
    if(!cityName.empty() && curl.get(cityUrl).requestOk() && parseWeather()) {
        spLog("notice") << "Weather ok for city " << cityName;
        weatherCache.put(cityCacheKey, {out.str()});
        return result;
    }
    if(curl.get(coordinatesUrl).requestOk() && parseWeather()) {
        spLog("notice") << "Weather ok for coordinates " << coordinates;
        weatherCache.put(coordinatesCacheKey, {out.str()});
        return result;
    }
    spLog("notice") << "curl results: " << curl.requestOk() << "-" << curl.httpResponseCode();
    return {};
}
