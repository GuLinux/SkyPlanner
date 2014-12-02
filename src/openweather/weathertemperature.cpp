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
#include "weathertemperature.h"
using namespace Wt;
using namespace std;
WeatherTemperature::WeatherTemperature(const Json::Object &json)
         : _day{json.get("day").toNumber().orIfNull(0.f)},
           _min{json.get("min").toNumber().orIfNull(0.f)},
           _max{json.get("max").toNumber().orIfNull(0.f)},
           _night{json.get("night").toNumber().orIfNull(0.f)},
           _eve{json.get("eve").toNumber().orIfNull(0.f)},
           _morn{json.get("morn").toNumber().orIfNull(0.f)}
{
}

/*
    Temperature _day;
    Temperature _min;
    Temperature _max;
    Temperature _night;
    Temperature _eve;
    Temperature _morn;
    */
