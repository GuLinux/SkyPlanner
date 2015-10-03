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
#include "weather.h"
#include <Wt/Json/Array>

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
using namespace Wt;
using namespace std;

Weather::Weather(const Json::Object &json)
    : _temperature(json.get("temp"))
{
    _dateGMT = boost::posix_time::from_time_t(json.get("dt").orIfNull(0));
    _pressure = json.get("pressure").orIfNull(0.d);
    _humidity = json.get("humidity").orIfNull(0);
    _windSpeed = json.get("speed").orIfNull(0.);
    _windAngle = Angle::degrees(json.get("deg").orIfNull(0));
    _clouds = json.get("clouds").orIfNull(0);
    _rain = json.get("clouds").orIfNull(0.);
    Json::Array weathers = json.get("weather");
    for(auto w: weathers) {
        _summaries.push_back({w});
    }
}

Weather::~Weather()
{
}
