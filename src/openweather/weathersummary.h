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
#ifndef WEATHERSUMMARY_H
#define WEATHERSUMMARY_H
#include <string>
#include <Wt/Json/Object>

/*
{
   "id":500,
   "main":"Rain",
   "description":"light rain",
   "icon":"10d"
}
*/

class WeatherSummary
{
public:
    WeatherSummary(const Wt::Json::Object &object);
    std::string main() { return _main; }
    std::string description() const { return _description; }
    std::string icon() const { return _icon; }
    std::string iconURL() const;
private:
    int _id;
    std::string _main;
    std::string _description;
    std::string _icon;

};

#endif // WEATHERSUMMARY_H
