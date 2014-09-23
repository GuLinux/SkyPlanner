#ifndef WEATHERFORECAST_H
#define WEATHERFORECAST_H
#include "utils/d_ptr.h"
#include "openweather/weathercity.h"
#include "openweather/weather.h"
#include <vector>

class WeatherForecast
{
public:
    WeatherForecast();
    ~WeatherForecast();
    std::string cod() const;
    std::shared_ptr<WeatherCity> city() const;
    int count() const;
    std::vector<std::shared_ptr<Weather>> weathers() const;
private:
    D_PTR;
};

#endif // WEATHERFORECAST_H
