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
#ifndef WEATHER_H
#define WEATHER_H

#include <boost/date_time.hpp>
#include "types.h"
#include <Wt/Json/Object>

#include "weathertemperature.h"
#include "weathersummary.h"

/*
{
         "dt":1411556400,
         "temp": {...},
         "pressure":884.41,
         "humidity":73,
         "weather":[
            {
               "id":500,
               "main":"Rain",
               "description":"light rain",
               "icon":"10d"
            }
         ],
         "speed":1.16,
         "deg":105,
         "clouds":68,
         "rain":0.5
      },
*/

class Weather
{
public:
    Weather(const Wt::Json::Object &json);
    ~Weather();
    boost::posix_time::ptime dateGMT() const { return _dateGMT; }
    WeatherTemperature temperature() const { return _temperature; }
    std::vector<WeatherSummary> summaries() const { return _summaries; }
    float pressure() const { return _pressure; }
    int humidity() const { return _humidity; }
    float windSpeed() const { return _windSpeed; }
    Angle windAngle() const { return _windAngle; }
    float rain() { return _rain; }
    int clouds() { return _clouds; }
private:
    boost::posix_time::ptime _dateGMT;
    WeatherTemperature _temperature;
    float _pressure;
    int _humidity;
    float _windSpeed;
    Angle _windAngle;
    float _rain;
    int _clouds;
    std::vector<WeatherSummary> _summaries;
};

#endif // WEATHER_H
