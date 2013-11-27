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

#include "rightascension.h"
using namespace std;
RightAscension::RightAscension(double decimal)
  : value(decimal)
{

}

RightAscension::RightAscension(const RightAscension::RA& sexagesimal)
  : value(static_cast<double>(sexagesimal.hours) + (sexagesimal.minutes/60.) + (sexagesimal.seconds/60./60.) )
{
}

RightAscension::operator double() const
{
  return value;
}
RightAscension::operator RA() const
{
  RA ra{static_cast<int>(value)};
  Degrees::Sexagesimal degrees = Degrees(value - ra.hours);
  ra.minutes = degrees.minutes;
  ra.seconds = degrees.seconds;
  return ra;
}

RightAscension::operator Degrees() const
{
  return Degrees{value*15.};
}

RightAscension::operator Radian() const
{
  Degrees asDegrees = *this;
  return asDegrees;
}