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
#include "types.h"

class Ephemeris
{
  public:
    struct RiseTransitSet {
      DateTime rise;
      DateTime transit;
      DateTime set;
    };
    struct LunarPhase {
      double illuminated_fraction;
      double position_angle;
      double phase_angle;
    };
    struct BestAltitude {
      Coordinates::AltAzimuth coordinates;
      DateTime when;
      RiseTransitSet rst;
      operator bool() const;
    };
    struct Darkness {
      DateTime begin;
      DateTime end;
      boost::posix_time::time_duration duration;
    };

    Ephemeris(const Coordinates::LatLng &geoPosition, const Timezone &timezone);
    ~Ephemeris();
    RiseTransitSet sun(const boost::gregorian::date &when, bool nightMode = true) const;
    RiseTransitSet astronomicalTwilight(const boost::gregorian::date &when, bool nightMode = true) const;
    RiseTransitSet moon(const boost::gregorian::date &when, bool nightMode = true) const;
    LunarPhase moonPhase(const boost::gregorian::date &when) const;
    Darkness darknessHours(const boost::gregorian::date &when) const;
    void setTimezone(const Timezone &timezone);
    Timezone timezone() const;

    BestAltitude findBestAltitude(const Coordinates::Equatorial &equatorial, const DateTime &rangeStart, const DateTime &rangeEnd) const;
  private:
    D_PTR;
};

#endif // EPHEMERIS_H
