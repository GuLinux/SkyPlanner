#include "openweather.h"
#include <Wt/Json/Parser>
#include <Wt/Json/Object>
#include "utils/curl.h"
#include "skyplanner.h"
#include <sstream>
#include <utils/format.h>

using namespace Wt;
using namespace std;

OpenWeather::OpenWeather()
{
}


shared_ptr<WeatherForecast> OpenWeather::forecast(const Coordinates::LatLng &coordinates, const std::string &cityName, int days)
{
    spLog("notice") << "Coordinates: " << coordinates << ", city name: " << cityName << ", days: " << days;
    string language = "en";
    string cityUrl = format("http://api.openweathermap.org/data/2.5/forecast/daily?q=%s&mode=json&units=metric&cnt=%d&type=like&lang=%s") % cityName % days % language;
    string coordinatesUrl = format("http://api.openweathermap.org/data/2.5/forecast/daily?lat=%f&lon=%f=json&units=metric&cnt=%d&type=like&lang=%s") % coordinates.latitude.degrees() % coordinates.longitude.degrees() % days % language;
    stringstream out;
    Curl curl(out);
    Json::Object weatherForecastObject;
    shared_ptr<WeatherForecast> result;
    auto parseWeather = [&out, &weatherForecastObject, &result] {
        Json::ParseError error;
        spLog("notice") << "got json string: '" << out.str() << "'";
        bool parsed = Json::parse(out.str(), weatherForecastObject, error);
        if(parsed) {
            spLog("notice") << "Parse was ok";
            result = make_shared<WeatherForecast>(weatherForecastObject);
            return result->cod() == "200";
        }
        return false;
    };
    if(!cityName.empty() && curl.get(cityUrl).requestOk() && parseWeather())
        return result;
    if(curl.get(coordinatesUrl).requestOk() && parseWeather())
        return result;
    spLog("notice") << "curl results: " << curl.requestOk() << "-" << curl.httpResponseCode();
    return {};
}
