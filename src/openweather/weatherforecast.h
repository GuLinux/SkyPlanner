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
