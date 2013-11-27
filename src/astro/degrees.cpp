/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Marco Gulino <email>
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

#include "degrees.h"
#include <math.h>
#include <cmath>

Degrees::Degrees( double degrees )
  : value(degrees)
{
}

Degrees::Sexagesimal::operator double() const
{
}

Degrees::Degrees( const Degrees::Sexagesimal &sexagesimal )
  : Degrees( static_cast<double>(sexagesimal.degrees) + (sexagesimal.minutes/60.) + (sexagesimal.seconds/60/60) )
{
}
Degrees::operator Radian() const
{
  return { value / 180. * M_PI};
}
Degrees::operator Sexagesimal() const
{
  Sexagesimal out{static_cast<int>(value)};
  out.minutes = (value-out.degrees) * 60;
  out.seconds = (((value-out.degrees) * 60) - out.minutes ) * 60;
  return out;
}

Degrees::operator double() const
{
  return value;
}
