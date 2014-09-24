#ifndef WEATHERFORECAST_H
#define WEATHERFORECAST_H
#include "openweather/weathercity.h"
#include "openweather/weather.h"
#include <vector>

#include <Wt/Json/Object>

class WeatherForecast
{
public:
    WeatherForecast(const Wt::Json::Object &json);
    ~WeatherForecast();
    std::string cod() const { return _cod; }
    WeatherCity city() const { return _city; }
    int count() const { return _count; }
    std::vector<Weather> weathers() const { return _weathers; }
private:
    std::string _cod;
    WeatherCity _city;
    int _count;
    std::vector<Weather> _weathers;
};

#endif // WEATHERFORECAST_H
