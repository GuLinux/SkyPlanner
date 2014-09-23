#ifndef WEATHERFORECAST_P_H
#define WEATHERFORECAST_P_H
#include "openweather/weatherforecast.h"

class WeatherForecast::Private {
public:
    std::string cod;
    std::shared_ptr<WeatherCity> city;
    int count;
    std::vector<std::shared_ptr<Weather>> weathers;
};

#endif // WEATHERFORECAST_P_H
