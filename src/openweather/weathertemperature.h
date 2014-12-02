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
#ifndef WEATHERTEMPERATURE_H
#define WEATHERTEMPERATURE_H
#include <Wt/Json/Object>

class WeatherTemperature
{
public:
    static constexpr float K2C = -273.15;
    struct Temperature {
        float _celsius;
        float kelvin() const { return _celsius - K2C; }
        float celsius() const { return _celsius; }
    };

    WeatherTemperature(const Wt::Json::Object &json);
    Temperature day() const { return _day; }
    Temperature min() const { return _min; }
    Temperature max() const { return _max; }
    Temperature night() const { return _night; }
    Temperature eve() const { return _eve; }
    Temperature morn() const { return _morn; }

private:
    Temperature _day;
    Temperature _min;
    Temperature _max;
    Temperature _night;
    Temperature _eve;
    Temperature _morn;
};

#endif // WEATHERTEMPERATURE_H
