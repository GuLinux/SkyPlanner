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
using namespace AAPlus;

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

Angle::Angle(double degrees) : _degrees(degrees), valid(true)
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

Angle::Sexagesimal Angle::sexagesimal() const
{
  double angle = degrees();
  int degrees_i = static_cast<int>(angle);
  angle -= degrees_i;
  angle *= angle>0?60.:-60.;
  int minutes_i = static_cast<int>(angle);
  angle -= minutes_i;
  angle *= 60.;
  return {degrees_i, minutes_i, angle};
}

Angle::Hours Angle::sexagesimalHours() const
{
  double angle = hours();
  int degrees_i = static_cast<int>(angle);
  angle -= degrees_i;
  angle *= angle>0?60.:-60.;
  int minutes_i = static_cast<int>(angle);
  angle -= minutes_i;
  angle *= 60.;
  return {degrees_i, minutes_i, angle};
}


string Angle::printable(Format format, PrintFormat printFormat) const
{
  if(format == Hourly) {
    double time = hours();
    int hours_i = static_cast<int>(time);

    time -= hours_i;
    time *= time>0?60.:-60.;

    int minutes_i = static_cast<int>(time);
    time -= minutes_i;
    time *= 60.;

    return (boost::format("%dh %2dm %.1fs") % hours_i % minutes_i % time ).str();
  }

  Sexagesimal asSexagesimal = sexagesimal();
  return (boost::format("%d%s %2d' %.1f\"") % asSexagesimal.degrees % (printFormat == HTML ? "&deg;" : "\302\260") % asSexagesimal.minutes % asSexagesimal.seconds ).str();
}

 
