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

#include "ephemeris.h"
#include "private/ephemeris_p.h"
#include "utils/d_ptr_implementation.h"
#include "aaplus/AA+.h"

using namespace std;
Ephemeris::Private::Private( const Ephemeris::GeoPosition &geoPosition, Ephemeris *q ) : geoPosition(geoPosition), q( q )
{
}

Ephemeris::Ephemeris( const Ephemeris::GeoPosition &geoPosition )
  : d( geoPosition, this )
{
}

Ephemeris::~Ephemeris()
{
}

Ephemeris::AltAzCoordinates Ephemeris::arDec2altAz( const pair<double,double> &arDec, const boost::posix_time::ptime &when ) const
{
  double AST = CAASidereal::ApparentGreenwichSiderealTime(d->date(when).Julian());
  double LongtitudeAsHourAngle = CAACoordinateTransformation::DegreesToHours(d->geoPosition.longitude);
  double LocalHourAngle = AST - LongtitudeAsHourAngle - CAACoordinateTransformation::RadiansToHours(arDec.first);
  auto coords = CAACoordinateTransformation::Equatorial2Horizontal(LocalHourAngle, CAACoordinateTransformation::RadiansToDegrees(arDec.second), d->geoPosition.latitude);
  return {coords.Y, CAACoordinateTransformation::MapTo0To360Range(180. + coords.X)};
}


Ephemeris::RiseTransitSet Ephemeris::moon( const boost::posix_time::ptime &when ) const
{
  CAARiseTransitSetDetails s = d->GetMoonRiseTransitSet(d->date(when).Julian(), d->geoPosition.longitude, d->geoPosition.latitude);
  return d->convert(s, when);
}

Ephemeris::RiseTransitSet Ephemeris::sun( const boost::posix_time::ptime &when ) const
{
  CAARiseTransitSetDetails s = d->GetSunRiseTransitSet(d->date(when).Julian(), d->geoPosition.longitude, d->geoPosition.latitude);
  return d->convert(s, when);
}

Ephemeris::LunarPhase Ephemeris::moonPhase( const boost::posix_time::ptime &when ) const
{
  LunarPhase lunarPhase;
  d->GetMoonIllumination(d->date(when).Julian(), lunarPhase.illuminated_fraction, lunarPhase.position_angle, lunarPhase.phase_angle);
  return lunarPhase;
}

Ephemeris::BestAltitude Ephemeris::findBestAltitude( const pair< double, double > &arDec, const boost::posix_time::ptime &rangeStart, const boost::posix_time::ptime &rangeEnd )
{
  BestAltitude bestAltitude{-90000000};
  for(boost::posix_time::ptime i = rangeStart; i<=rangeEnd; i+= boost::posix_time::time_duration(0, 10, 0)) {
    AltAzCoordinates c = arDec2altAz(arDec, i);
    if(c.altitude > bestAltitude.altitude)
    bestAltitude = BestAltitude{c.altitude, i};
  }
  return bestAltitude;
}


CAADate Ephemeris::Private::date( const boost::posix_time::ptime &when ) const
{
  // TODO: use fractional seconds?
  return {when.date().year(), when.date().month(), when.date().day(), when.time_of_day().hours(), when.time_of_day().minutes(), when.time_of_day().seconds(), true};
}
boost::posix_time::ptime Ephemeris::Private::date( const CAADate &when ) const
{
  return {{when.Year(), when.Month(), when.Day()}, {when.Hour(), when.Minute(), when.Second()}};
}

Ephemeris::RiseTransitSet Ephemeris::Private::convert( const CAARiseTransitSetDetails &details, const boost::posix_time::ptime &when )
{
  double JD = date(when).Julian();
  auto toJd = [JD](double t) {
    return (JD + (t / 24.00));
  };
  return { date(CAADate(toJd(details.Rise), true)), date(CAADate(toJd(details.Transit), true)), date(CAADate(toJd(details.Set), true)) };
}


void Ephemeris::Private::GetSolarRaDecByJulian(double JD, double& RA, double& Dec)
{
  double JDSun = CAADynamicalTime::UTC2TT(JD);
  double lambda = CAASun::ApparentEclipticLongitude(JDSun);
  double beta = CAASun::ApparentEclipticLatitude(JDSun);
  double epsilon = CAANutation::TrueObliquityOfEcliptic(JD);
  CAA2DCoordinate Solarcoord = CAACoordinateTransformation::Ecliptic2Equatorial(lambda, beta, epsilon);
  RA = Solarcoord.X;
  Dec = Solarcoord.Y;
}

void Ephemeris::Private::GetLunarRaDecByJulian(double JD, double& RA, double& Dec)
{
  double JDMoon = CAADynamicalTime::UTC2TT(JD);
  double lambda = CAAMoon::EclipticLongitude(JDMoon);
  double beta = CAAMoon::EclipticLatitude(JDMoon);
  double epsilon = CAANutation::TrueObliquityOfEcliptic(JD);
  CAA2DCoordinate Lunarcoord = CAACoordinateTransformation::Ecliptic2Equatorial(lambda, beta, epsilon);
  RA = Lunarcoord.X;
  Dec = Lunarcoord.Y;
}

CAARiseTransitSetDetails Ephemeris::Private::GetSunRiseTransitSet(double JD, double longitude, double latitude)
{
  double alpha1 = 0;
  double delta1 = 0;
  GetSolarRaDecByJulian(JD - 1, alpha1, delta1);
  double alpha2 = 0;
  double delta2 = 0;
  GetSolarRaDecByJulian(JD, alpha2, delta2);
  double alpha3 = 0;
  double delta3 = 0;
  GetSolarRaDecByJulian(JD + 1, alpha3, delta3);
  return CAARiseTransitSet::Calculate(CAADynamicalTime::UTC2TT(JD), alpha1, delta1, alpha2, delta2, alpha3, delta3, longitude, latitude, -0.8333);
}

CAARiseTransitSetDetails Ephemeris::Private::GetMoonRiseTransitSet(double JD, double longitude, double latitude)
{
  double alpha1 = 0;
  double delta1 = 0;
  GetLunarRaDecByJulian(JD - 1, alpha1, delta1);
  double alpha2 = 0;
  double delta2 = 0;
  GetLunarRaDecByJulian(JD, alpha2, delta2);
  double alpha3 = 0;
  double delta3 = 0;
  GetLunarRaDecByJulian(JD + 1, alpha3, delta3);
  return CAARiseTransitSet::Calculate(CAADynamicalTime::UTC2TT(JD), alpha1, delta1, alpha2, delta2, alpha3, delta3, longitude, latitude, 0.125);
}

void Ephemeris::Private::GetMoonIllumination(double JD, double& illuminated_fraction, double& position_angle, double& phase_angle)
{
  double moon_alpha = 0;
  double moon_delta = 0;
  GetLunarRaDecByJulian(JD, moon_alpha, moon_delta);
  double sun_alpha = 0;
  double sun_delta = 0;
  GetSolarRaDecByJulian(JD, sun_alpha, sun_delta);
  double geo_elongation = CAAMoonIlluminatedFraction::GeocentricElongation(moon_alpha, moon_delta, sun_alpha, sun_delta);

  position_angle = CAAMoonIlluminatedFraction::PositionAngle(sun_alpha, sun_delta, moon_alpha, moon_delta);
  phase_angle = CAAMoonIlluminatedFraction::PhaseAngle(geo_elongation, 368410.0, 149971520.0);
  illuminated_fraction = CAAMoonIlluminatedFraction::IlluminatedFraction(phase_angle);
}
