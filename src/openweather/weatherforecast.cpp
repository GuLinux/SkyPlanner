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
#include "weatherforecast.h"
#include <Wt/Json/Array>
using namespace std;
using namespace Wt;

/*
{
   "cod":"200",
   "message":0.0167,
   "city":{... },
      "country":"JP",
      "population":0
   },
   "cnt":34,
   "list":[  ]
}
*/

WeatherForecast::WeatherForecast(const Json::Object &json)
    : _city(json.get("city"))
{
    _count = json.get("cnt").toNumber().orIfNull(0);
    _cod = json.get("cod").toString().orIfNull("");
    Json::Array weathers = json.get("list");
    for(auto w: weathers) {
        _weathers.push_back({w});
    }
}

WeatherForecast::~WeatherForecast()
{
}
