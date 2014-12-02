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
#ifndef WEATHERCITY_H
#define WEATHERCITY_H
#include "types.h"
#include <Wt/Json/Object>

class WeatherCity
{
public:
    WeatherCity(const Wt::Json::Object &json);
    ~WeatherCity();
    uint64_t id() const { return _id; }
    std::string name() const { return _name; }
    Coordinates::LatLng coordinates() const { return _coordinates; }
    std::string country() const { return _country; }
    uint64_t population() const { return _population; }
private:
    u_int64_t _id;
    std::string _name;
    Coordinates::LatLng _coordinates;
    std::string _country;
    uint64_t _population;
};

#endif // WEATHERCITY_H
