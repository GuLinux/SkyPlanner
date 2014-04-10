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

#ifndef EPHEMERIS_P_H
#define EPHEMERIS_P_H
#include "ephemeris.h"
#include <libnova/libnova.h>
#include <mutex>

typedef std::function<double(double, ln_lnlat_posn*,ln_rst_time*)> RiseTransitSetFunction;
class Ephemeris::Private
{
  public:
    Private( const Coordinates::LatLng &geoPosition, const Timezone &timezone, Ephemeris *q );
    Coordinates::LatLng geoPosition;

    double dateToJulian(const boost::posix_time::ptime &date, bool utc = false) const;
    DateTime julianToDate(double jd) const;

    Ephemeris::RiseTransitSet rst(const boost::gregorian::date &date, RiseTransitSetFunction, bool nightMode);
    Ephemeris::RiseTransitSet rst(const boost::posix_time::ptime &when, RiseTransitSetFunction, bool nightMode);

    ln_lnlat_posn lnGeoPosition() const;
    Timezone timezone;
    std::unique_lock<std::mutex> mutexLock;
  private:
    class Ephemeris *const q;
};

#endif // EPHEMERIS_P_H
