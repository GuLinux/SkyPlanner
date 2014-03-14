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
#include "aaplus/AA+.h"

class Ephemeris::Private
{
  public:
    Private( const Coordinates::LatLng &geoPosition, Ephemeris *q );
    Coordinates::LatLng geoPosition;
    void GetSolarRaDecByJulian(double JD, double& RA, double& Dec);
    void GetLunarRaDecByJulian(double JD, double& RA, double& Dec);
    AAPlus::CAARiseTransitSetDetails GetSunRiseTransitSet(double JD, double longitude, double latitude, double sunAltitude = -0.8333);
    AAPlus::CAARiseTransitSetDetails GetMoonRiseTransitSet(double JD, double longitude, double latitude);
    void GetMoonIllumination(double JD, double& illuminated_fraction, double& position_angle, double& phase_angle);

    AAPlus::CAADate date(const boost::posix_time::ptime &when) const;
    boost::posix_time::ptime date(const AAPlus::CAADate &when) const;
    Ephemeris::RiseTransitSet convert(const AAPlus::CAARiseTransitSetDetails &details, const boost::posix_time::ptime &when);
  private:
    class Ephemeris *const q;
};

#endif // EPHEMERIS_P_H
