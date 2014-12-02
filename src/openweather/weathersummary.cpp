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
#include "weathersummary.h"
#include "utils/format.h"

using namespace Wt;
using namespace std;
/*
{
   "id":500,
   "main":"Rain",
   "description":"light rain",
   "icon":"10d"
}
*/


WeatherSummary::WeatherSummary(const Json::Object &object)
{
    _id = object.get("id").toNumber().orIfNull(0);
    _main = object.get("main").toString().orIfNull("");
    _description = object.get("description").toString().orIfNull("");
    _icon = object.get("icon").toString().orIfNull("");
}

string WeatherSummary::iconURL() const {
    return format("http://openweathermap.org/img/w/%s.png") % icon();
}
