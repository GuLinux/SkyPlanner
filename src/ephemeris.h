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

#ifndef EPHEMERIS_H
#define EPHEMERIS_H

#include "utils/d_ptr.h"
#include <boost/date_time.hpp>

class Ephemeris
{
  public:
    struct GeoPosition {
      double latitude;
      double longitude;
    };
    
    struct RiseTransitSet {
      boost::posix_time::ptime rise;
      boost::posix_time::ptime transit;
      boost::posix_time::ptime set;
    };
    struct LunarPhase {
      double illuminated_fraction;
      double position_angle;
      double phase_angle;
    };
    struct AltAzCoordinates {
      double altitude;
      double azimuth;
    };
    struct BestAltitude {
      double altitude;
      boost::posix_time::ptime when;
    };
    Ephemeris(const GeoPosition &geoPosition);
    ~Ephemeris();
    RiseTransitSet sun(const boost::posix_time::ptime &when) const;
    RiseTransitSet moon(const boost::posix_time::ptime &when) const;
    LunarPhase moonPhase(const boost::posix_time::ptime &when) const;

    AltAzCoordinates arDec2altAz(const std::pair<double,double> &arDec, const boost::posix_time::ptime &when) const;
    BestAltitude findBestAltitude(const std::pair<double,double> &arDec, const boost::posix_time::ptime &rangeStart, const boost::posix_time::ptime &rangeEnd);
  private:
    D_PTR;
};

#endif // EPHEMERIS_H
