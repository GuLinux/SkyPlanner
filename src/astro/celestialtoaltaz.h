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

#ifndef CELESTIALTOALTAZ_H
#define CELESTIALTOALTAZ_H
#include "radian.h"
#include "rightascension.h"
#include <boost/date_time.hpp>

class CelestialToAltAz
{
public:
  struct LatLng {
    Degrees latitude;
    Degrees longitude;
  };
  struct AltAzCoordinates {
    Radian alt;
    Radian azimuth;
  };
  struct RADecCoordinates {
    Radian rightAscension;
    Radian declination;
  };
  CelestialToAltAz(const LatLng &latlng);
  double greenwichSideralTime(const boost::posix_time::ptime &utc);
  double localSideralTime(const boost::posix_time::ptime &utc);
  AltAzCoordinates coordinatesAt(const RADecCoordinates &raDecCoordinates, const boost::posix_time::ptime &when);
  
private:
  const LatLng latlng;
};

#endif // CELESTIALTOALTAZ_H
