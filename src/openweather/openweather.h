#ifndef OPENWEATHER_H
#define OPENWEATHER_H

#include "weatherforecast.h"

class OpenWeather
{
public:
    OpenWeather();
    WeatherForecast forecast(const Coordinates::LatLng &coordinates, const std::string &cityName = {}, int days = 16);
};

#endif // OPENWEATHER_H
