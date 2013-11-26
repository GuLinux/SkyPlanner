/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  <copyright holder> <email>
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

#ifndef FORECAST_P_H
#define FORECAST_P_H
#include "forecast.h"
#include <Wt/Http/Client>

namespace Wt {
class WApplication;
}

class Forecast::Private
{
public:
    Private(Forecast* q);
    Wt::Http::Client client;
    Wt::WApplication *app;
    void parseForecast(const std::string &forecast);
private:
    class Forecast* const q;
};
#endif // FORECAST_P_H
