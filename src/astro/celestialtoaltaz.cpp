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

#include "celestialtoaltaz.h"
#include <list>
using namespace std;
typedef boost::posix_time::ptime ptime;
CelestialToAltAz::CelestialToAltAz(const CelestialToAltAz::LatLng& latlng) : latlng(latlng)
{
}

double julianDate(const boost::posix_time::ptime& when)
{
  int jd = when.date().julian_day();
  boost::posix_time::ptime noon = when - when.time_of_day() + boost::posix_time::time_duration(12, 0, 0);
  boost::posix_time::time_duration delta = when - noon;
  return static_cast<double>(jd) + delta.total_seconds() / 60. /60. / 24.;
}


double CelestialToAltAz::greenwichSideralTime(const boost::posix_time::ptime& utc)
{
  double jd = julianDate({utc.date(), {0,0,0}});
  double time = static_cast<double>(utc.time_of_day().total_seconds()) / 60. / 60.;
  double t=(jd-2451545.0)/36525.0;
  double t0 = 6.697374558 + (2400.051336*t)+(0.000025862*pow(t, 2.0))+(time*1.0027379093);
  while(t0 > 24)
    t0 -= 24.0;
  while(t0 < 0)
    t0 += 24.;
  return t0;
}

double CelestialToAltAz::localSideralTime(const boost::posix_time::ptime& utc)
{
  double lst = greenwichSideralTime(utc) + static_cast<double>(latlng.longitude);
  while(lst > 24)
    lst -= 24.0;
  while(lst < 0)
    lst += 24.;
  return lst;
}


CelestialToAltAz::AltAzCoordinates CelestialToAltAz::coordinatesAt(const CelestialToAltAz::RADecCoordinates& raDecCoordinates, const boost::posix_time::ptime& when)
{
  double lst = localSideralTime(when);
  double localHourAngle = lst - raDecCoordinates.rightAscension;
  // tan A = sin H / (cos H sin f - tan d cos f )
  // sin h = sin f sin d + cos f cos d cos H
  double tanAzimuth = sin(localHourAngle) / (cos(localHourAngle) * sin(latlng.latitude) - tan(raDecCoordinates.declination)*cos(latlng.latitude));
  double sinAltitude = sin(latlng.latitude) * sin(raDecCoordinates.declination) + cos(latlng.latitude)*cos(raDecCoordinates.declination) * cos(localHourAngle);
  return { asin(sinAltitude), atan(tanAzimuth) };
}
