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
#include <libnova/libnova.h>

using namespace std;
static mutex ephemerisMutex;
Ephemeris::Private::Private( const Coordinates::LatLng &geoPosition, const Timezone &timezone, Ephemeris *q ) : geoPosition(geoPosition), timezone(timezone), mutexLock(ephemerisMutex), q( q )
{
}

Ephemeris::Ephemeris( const Coordinates::LatLng &geoPosition, const Timezone &timezone )
  : d( geoPosition, timezone, this )
{
}

Ephemeris::~Ephemeris()
{
}

void Ephemeris::setTimezone(const Timezone &timezone)
{
  d->timezone = timezone;
}


Ephemeris::RiseTransitSet Ephemeris::moon( const boost::gregorian::date &when, bool nightMode ) const
{
  //DateTime dtWhen = DateTime::fromLocal(when, d->timezone);
  return d->rst(when, [](double jd, ln_lnlat_posn* pos,ln_rst_time* rst){ return ln_get_lunar_rst(jd, pos, rst); }, nightMode);
}


Ephemeris::RiseTransitSet Ephemeris::sun( const boost::gregorian::date &when, bool nightMode ) const
{
  return d->rst(when, [](double jd, ln_lnlat_posn* pos,ln_rst_time* rst){ return ln_get_solar_rst(jd, pos, rst); }, nightMode);
}

Ephemeris::RiseTransitSet Ephemeris::astronomicalTwilight( const boost::gregorian::date &when, bool nightMode ) const
{
  return d->rst(when, [](double jd, ln_lnlat_posn* pos,ln_rst_time* rst){ return ln_get_solar_rst_horizon(jd, pos, LN_SOLAR_ASTRONOMICAL_HORIZON, rst); }, nightMode);
}


Ephemeris::LunarPhase Ephemeris::moonPhase( const boost::gregorian::date &date ) const
{
  boost::posix_time::ptime when(date);
  LunarPhase lunarPhase;
  lunarPhase.illuminated_fraction = ln_get_lunar_disk(d->dateToJulian(when));
  lunarPhase.phase_angle = ln_get_lunar_phase(d->dateToJulian(when));
  lunarPhase.position_angle = ln_get_lunar_bright_limb(d->dateToJulian(when));
  return lunarPhase;
}


Ephemeris::Darkness Ephemeris::darknessHours( const boost::gregorian::date &when ) const
{
  RiseTransitSet darkness = astronomicalTwilight(when);
  RiseTransitSet _moon = moon(when);
  
  if(darkness.set < _moon.set)
    darkness.set = _moon.set;

  if(darkness.rise > _moon.rise)
    darkness.rise = _moon.rise;
  return {darkness.set, darkness.rise, darkness.rise.utc - darkness.set.utc};
}

Timezone Ephemeris::timezone() const
{
  return d->timezone;
}

Ephemeris::BestAltitude Ephemeris::findBestAltitude( const Coordinates::Equatorial &arDec, const boost::posix_time::ptime &rangeStart, const boost::posix_time::ptime &rangeEnd, const boost::posix_time::time_duration steps) const
{
  ln_equ_posn object{arDec.rightAscension.degrees(), arDec.declination.degrees()};
  RiseTransitSet rst = d->rst(rangeStart, [&object](double jd, ln_lnlat_posn* pos,ln_rst_time* rst){
    int result = 0;
    int horizon = 0;
    result = ln_get_object_next_rst(jd, pos, &object, rst);
// TODO: hack, but it works. for now...
    while(result == 1) {
      result = ln_get_object_next_rst_horizon(jd, pos, &object, horizon, rst);
      horizon += 5;
    }
    return result;
  }, false);
  if(rst.transit.utc == boost::posix_time::ptime{})  return {};

  ln_lnlat_posn observer = d->lnGeoPosition();

  auto bestAltitude = [this,&observer, &object](const boost::posix_time::ptime &when) mutable {
    ln_hrz_posn position;
    ln_get_hrz_from_equ(&object, &observer, d->dateToJulian(when), &position);
    return BestAltitude{{ Angle::degrees(position.alt), Angle::degrees(position.az) }, when };
  };

  // TODO: fix, using utc
  if(rst.transit.localtime < d->timezone.fix(rangeEnd) && rst.transit.localtime > d->timezone.fix(rangeStart)) {
    return bestAltitude(rst.transit.localtime - boost::posix_time::seconds(d->timezone.dstOffset) );
  }

  BestAltitude atStart = bestAltitude(rangeStart);
  BestAltitude atEnd = bestAltitude(rangeEnd);
  return atStart.coordinates.altitude.degrees() > atEnd.coordinates.altitude.degrees() ? atStart : atEnd;
}

Ephemeris::BestAltitude::operator bool() const
{
  return when != boost::posix_time::ptime{} && coordinates.altitude.valid() && coordinates.azimuth.valid();
}



double Ephemeris::Private::dateToJulian(const boost::posix_time::ptime &when, bool utc) const
{
/*  tm _tm = boost::posix_time::to_tm(when);
  time_t t = mktime(&_tm);
  return ln_get_julian_from_timet(&t);
*/
  // TODO: always UFC as input?
  if(utc) {
    ln_date date{when.date().year(), when.date().month(), when.date().day(), when.time_of_day().hours(), when.time_of_day().minutes(), static_cast<double>(when.time_of_day().seconds())};
    return ln_get_julian_day(&date);
  }
  ln_zonedate zonedate{when.date().year(), when.date().month(), when.date().day(), when.time_of_day().hours(), when.time_of_day().minutes(), static_cast<double>(when.time_of_day().seconds()), timezone.rawOffset};
  ln_date date;
  ln_zonedate_to_date(&zonedate, &date);
  return ln_get_julian_day(&date);
}

DateTime Ephemeris::Private::julianToDate(double jd) const
{
  // TODO: local/UTC date support
  time_t utc_time;
  ln_get_timet_from_julian(jd, &utc_time);
  try {
    boost::posix_time::ptime utc = boost::posix_time::from_time_t(utc_time);
    return DateTime::fromUTC(utc, timezone); 
  } catch(std::exception &e) {
    return {};
  }
}


ln_lnlat_posn Ephemeris::Private::lnGeoPosition() const
{
  return {geoPosition.longitude.degrees(), geoPosition.latitude.degrees()};
}

Ephemeris::RiseTransitSet Ephemeris::Private::rst(const boost::gregorian::date &date, RiseTransitSetFunction f, bool nightMode)
{
  return rst(boost::posix_time::ptime(date), f, nightMode);
}

Ephemeris::RiseTransitSet Ephemeris::Private::rst(const boost::posix_time::ptime &when, RiseTransitSetFunction f, bool nightMode)
{
  ln_lnlat_posn where = lnGeoPosition();
  ln_rst_time rst;
  f(dateToJulian(when), &where, &rst);
  if(nightMode && rst.rise < rst.set) {
    ln_rst_time nextDay_rst;
    f(dateToJulian(when + boost::posix_time::hours{24}), &where, &nextDay_rst);
    return {julianToDate(nextDay_rst.rise), julianToDate(rst.transit), julianToDate(rst.set) };
  }
  return {julianToDate(rst.rise), julianToDate(rst.transit), julianToDate(rst.set) };
}





