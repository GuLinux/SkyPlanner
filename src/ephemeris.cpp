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
using namespace AAPlus;
Ephemeris::Private::Private( const Coordinates::LatLng &geoPosition, Ephemeris *q ) : geoPosition(geoPosition), q( q )
{
}

Ephemeris::Ephemeris( const Coordinates::LatLng &geoPosition )
  : d( geoPosition, this )
{
}

Ephemeris::~Ephemeris()
{
}

Coordinates::AltAzimuth Ephemeris::arDec2altAz( const Coordinates::Equatorial &arDec, const boost::posix_time::ptime &when ) const
{
  double AST = CAASidereal::ApparentGreenwichSiderealTime(d->date(when).Julian());
  double LongtitudeAsHourAngle = CAACoordinateTransformation::DegreesToHours(d->geoPosition.longitude.degrees());
  double LocalHourAngle = AST - LongtitudeAsHourAngle - arDec.rightAscension.hours();
  auto coords = CAACoordinateTransformation::Equatorial2Horizontal(LocalHourAngle, arDec.declination.degrees(), d->geoPosition.latitude.degrees());
  return {Angle::degrees(coords.Y), Angle::degrees(180. + coords.X)};
}


Ephemeris::RiseTransitSet Ephemeris::moon( const boost::posix_time::ptime &when, bool nightMode ) const
{
  CAARiseTransitSetDetails s = d->GetMoonRiseTransitSet(d->date(when).Julian(), d->geoPosition.longitude.degrees(), d->geoPosition.latitude.degrees());
  auto result =  d->convert(s, when);
  if(nightMode && result.set > result.rise) {
    auto nextDay = when + boost::posix_time::hours(24);
    s = d->GetMoonRiseTransitSet(d->date(nextDay).Julian(), d->geoPosition.longitude.degrees(), d->geoPosition.latitude.degrees());
    auto nextDayResult =  d->convert(s, nextDay);
    result.rise = nextDayResult.rise;
  }
  return result;
}


Ephemeris::RiseTransitSet Ephemeris::sun( const boost::posix_time::ptime &when, bool nightMode ) const
{
  boost::posix_time::ptime midnight(when.date());
  CAARiseTransitSetDetails s = d->GetSunRiseTransitSet(d->date(midnight).Julian(), d->geoPosition.longitude.degrees(), d->geoPosition.latitude.degrees());
  auto result =  d->convert(s, midnight);
  if(nightMode && result.rise < result.set) {
    auto nextDay = midnight + boost::posix_time::hours(24);
    s = d->GetSunRiseTransitSet(d->date(nextDay).Julian(), d->geoPosition.longitude.degrees(), d->geoPosition.latitude.degrees());
    auto nextDayResult =  d->convert(s, nextDay);
    result.rise = nextDayResult.rise;
  }
  return result;
}

Ephemeris::RiseTransitSet Ephemeris::astronomicalTwilight( const boost::posix_time::ptime &when, bool nightMode ) const
{
  boost::posix_time::ptime midnight(when.date());
  CAARiseTransitSetDetails s = d->GetSunRiseTransitSet(d->date(midnight).Julian(), d->geoPosition.longitude.degrees(), d->geoPosition.latitude.degrees(), -18);
  auto result =  d->convert(s, midnight);
  if(nightMode && result.rise < result.set) {
    auto nextDay = midnight + boost::posix_time::hours(24);
    s = d->GetSunRiseTransitSet(d->date(nextDay).Julian(), d->geoPosition.longitude.degrees(), d->geoPosition.latitude.degrees(), -18);
    auto nextDayResult =  d->convert(s, nextDay);
    result.rise = nextDayResult.rise;
  }
  return result;
}

Ephemeris::LunarPhase Ephemeris::moonPhase( const boost::posix_time::ptime &when ) const
{
  LunarPhase lunarPhase;
  d->GetMoonIllumination(d->date(when).Julian(), lunarPhase.illuminated_fraction, lunarPhase.position_angle, lunarPhase.phase_angle);
  return lunarPhase;
}


Ephemeris::Darkness Ephemeris::darknessHours( const boost::posix_time::ptime &when ) const
{
  RiseTransitSet darkness = astronomicalTwilight(when);
  RiseTransitSet _moon = moon(when);
  
  if(darkness.set < _moon.set)
    darkness.set = _moon.set;

  if(darkness.rise > _moon.rise)
    darkness.rise = _moon.rise;

/*
  if(moonEphemeris.set - darkness.set < boost::posix_time::hours(4) && moonEphemeris.set > darkness.set )
    darkness.set = moonEphemeris.set;

  if(darkness.rise - moonEphemeris.rise < boost::posix_time::hours(4) && moonEphemeris.rise < darkness.rise )
    darkness.rise = moonEphemeris.rise;
*/
  return {darkness.set, darkness.rise, darkness.rise - darkness.set};
}

Ephemeris::BestAltitude Ephemeris::findBestAltitude( const Coordinates::Equatorial &arDec, const boost::posix_time::ptime &rangeStart, const boost::posix_time::ptime &rangeEnd ) const
{
  Ephemeris::BestAltitude bestAltitude;
  for(boost::posix_time::ptime i = rangeStart; i<=rangeEnd; i+= boost::posix_time::time_duration(0, 10, 0)) {
    Coordinates::AltAzimuth c = arDec2altAz(arDec, i);
    if( ! bestAltitude.coordinates || c.altitude.degrees() > bestAltitude.coordinates.altitude.degrees() )
      bestAltitude = {c,i};
  }
  return bestAltitude;
}


CAADate Ephemeris::Private::date( const boost::posix_time::ptime &when ) const
{
  // TODO: use fractional seconds?
  return {
    when.date().year(), when.date().month(),
    static_cast<double>(when.date().day()), 
    static_cast<double>(when.time_of_day().hours()),
    static_cast<double>(when.time_of_day().minutes()),
    static_cast<double>(when.time_of_day().seconds()),
    true};
}
boost::posix_time::ptime Ephemeris::Private::date( const CAADate &when ) const
{
  return {
    { 
      static_cast<uint16_t>(when.Year()),
      static_cast<uint16_t>(when.Month()),
      static_cast<uint16_t>(when.Day())
    },
    {
      static_cast<boost::posix_time::time_duration::hour_type>(when.Hour()),
      static_cast<boost::posix_time::time_duration::min_type>(when.Minute()),
      static_cast<boost::posix_time::time_duration::sec_type>(when.Second())
    }
  };
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

CAARiseTransitSetDetails Ephemeris::Private::GetSunRiseTransitSet(double JD, double longitude, double latitude, double sunAltitude)
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
  return CAARiseTransitSet::Calculate(CAADynamicalTime::UTC2TT(JD), alpha1, delta1, alpha2, delta2, alpha3, delta3, longitude, latitude, sunAltitude);
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
