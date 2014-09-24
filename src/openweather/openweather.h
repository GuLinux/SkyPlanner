#ifndef OPENWEATHER_H
#define OPENWEATHER_H

#include "weatherforecast.h"
#include <memory>

class OpenWeather
{
public:
    OpenWeather();
    std::shared_ptr<WeatherForecast> forecast(const Coordinates::LatLng &coordinates, const std::string &cityName = {}, int days = 16);
};

#endif // OPENWEATHER_H
