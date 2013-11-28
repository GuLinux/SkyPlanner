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

#include "types.h"
#include "aaplus/AA+.h"
#include <string>
#include <boost/format.hpp>

using namespace std;

Angle Angle::degrees(double degrees)
{
  return Angle(degrees);
}

Angle Angle::radians(double radians)
{
  return Angle::degrees(CAACoordinateTransformation::RadiansToDegrees(radians) );
}

Angle Angle::hours(double hours)
{
  return Angle::degrees(CAACoordinateTransformation::HoursToDegrees(hours));
}

Angle::Angle(double degrees) : _degrees(CAACoordinateTransformation::MapTo0To360Range(degrees)), valid(true)
{
}

Angle::Angle() : _degrees(0), valid(false)
{
}

Angle::operator bool() const
{
  return valid;
}

double Angle::degrees() const
{
  return _degrees;
}

double Angle::radians() const
{
  return CAACoordinateTransformation::DegreesToRadians(degrees());
}

double Angle::hours() const
{
  return CAACoordinateTransformation::DegreesToHours(degrees());
}

string Angle::printable(Format format) const
{
  if(format == Hourly) {
    double time = hours();
    int hours_i = static_cast<int>(time);
    time -= hours_i;
    time *= 60.;
    int minutes_i = static_cast<int>(time);
    time -= minutes_i;
    time *= 60.;
    return (boost::format("%dh %d2m %.3fs") % hours_i % minutes_i % time ).str();
  }

  double angle = degrees();
  int degrees_i = static_cast<int>(angle);
  angle -= degrees_i;
  angle *= 60.;
  int minutes_i = static_cast<int>(angle);
  angle -= minutes_i;
  angle *= 60.;
  return (boost::format("%d\302\260 %d2' %.3f\"") % degrees_i % minutes_i % angle ).str();
}

 
