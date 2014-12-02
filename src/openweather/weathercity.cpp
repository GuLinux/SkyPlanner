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
#include "weathercity.h"

/*
   "city":{
      "id":1851632,
      "name":"Shuzenji",
      "coord":{
         "lon":138.933334,
         "lat":34.966671
      },
      "country":"JP",
      "population":0
   },
*/

using namespace std;
using namespace Wt;

WeatherCity::WeatherCity(const Json::Object &json)
{
    _id = json.get("id").toNumber().orIfNull(0);
    _name = json.get("name").toString().orIfNull("");
    Json::Object latlng = json.get("coord");
    _coordinates.latitude = Angle::degrees(latlng.get("lat").toNumber().orIfNull(0.0));
    _coordinates.longitude = Angle::degrees(latlng.get("lon").toNumber().orIfNull(0.0));
    _country = json.get("country").toString().orIfNull("");
    _population = json.get("population").toNumber().orIfNull(0);
}

WeatherCity::~WeatherCity()
{
}
