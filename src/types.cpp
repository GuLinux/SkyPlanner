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
#include <string>
#include <boost/format.hpp>
#include "utils/format.h"
#include <libnova/libnova.h>
#ifndef TESTS_NO_WT
#include <Wt/Json/Value>
#include <Wt/Json/Object>
#include <Wt/Json/Parser>
using namespace Wt;
#endif
using namespace std;

Angle Angle::degrees(double degrees)
{
  return Angle(degrees);
}

Angle Angle::arcMinutes(double arcMinutes)
{
  return Angle( arcMinutes / 60. );
}

Angle Angle::radians(double radians)
{
  return Angle::degrees( ln_rad_to_deg(radians) );
}

Angle Angle::hours(double hours)
{
  return Angle::degrees(hours*360./24.);
}

Angle::Angle(double degrees) : _degrees(degrees), _valid(true)
{
}

Angle::Angle() : _degrees(0), _valid(false)
{
}

bool Angle::valid() const
{
  return _valid;
}


Angle Angle::operator*(double coefficient) const
{
  return Angle::degrees(degrees() * coefficient);
}

double Angle::degrees() const
{
  return _degrees;
}

double Angle::arcMinutes() const
{
  return degrees() * 60.;
}

double Angle::radians() const
{
  return ln_deg_to_rad(degrees());
}

double Angle::hours() const
{
  return degrees() * 24. / 360.;
}

Angle::Sexagesimal Angle::sexagesimal() const
{
  double angle = degrees();
  int degrees_i = static_cast<int>(angle);
  angle -= degrees_i;
  angle *= angle>0?60.:-60.;
  angle = round(angle * 1000.) / 1000.;
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
  angle = round(angle * 1000.) / 1000.;
  int minutes_i = static_cast<int>(angle);
  angle -= minutes_i;
  angle *= 60.;
  return {degrees_i, minutes_i, angle};
}


string Angle::printable(Format format, PrintFormat printFormat) const
{
  if(format == Hourly) {
    auto sHours = sexagesimalHours();
    return (boost::format("%dh %2dm %.1fs") % sHours.hours % sHours.minutes % sHours.seconds ).str();

  }

  Sexagesimal asSexagesimal = sexagesimal();
  return (boost::format("%d%s %2d' %.1f\"") % asSexagesimal.degrees % (printFormat == HTML ? "&deg;" : "\302\260") % asSexagesimal.minutes % asSexagesimal.seconds ).str();
}

 

boost::posix_time::ptime Timezone::fix(const boost::posix_time::ptime &src) const
{
  return src + boost::posix_time::seconds(dstOffset);
}

boost::posix_time::ptime Timezone::fixUTC(const boost::posix_time::ptime &src) const
{
  return fix(src) + boost::posix_time::seconds(dstOffset);
}

string Timezone::key(const boost::posix_time::ptime &when, const std::string &language)
{
  return key(latitude, longitude, when, language);
}
string Timezone::key(double latitude, double longitude, const boost::posix_time::ptime &when, const string &language)
{
  return format("%f-%f-%s-%s") % latitude % longitude % boost::posix_time::to_iso_extended_string(boost::posix_time::ptime(when.date())) % language;
}

Timezone Timezone::from(const string &response, double lat, double lng)
{
  Timezone timezone;
  #ifndef TESTS_NO_WT
  Json::Object timezoneJsonObject;
  Json::parse(response, timezoneJsonObject);
  timezone.dstOffset = timezoneJsonObject.get("dstOffset");
  timezone.rawOffset = timezoneJsonObject.get("rawOffset");
  timezone.timeZoneId = timezoneJsonObject.get("timeZoneId").orIfNull(string{});
  timezone.timeZoneName = timezoneJsonObject.get("timeZoneName").orIfNull(string{});
  timezone.latitude = lat;
  timezone.longitude = lng;
  return timezone;
#endif
}

ostream &operator<<(ostream &o, const Timezone &t)
{
  o << "{ dstOffset=" << t.dstOffset << ", rawOffset=" << t.rawOffset << ", timeZoneId=" << t.timeZoneId << ", timeZoneName=" << t.timeZoneName << ", latitude=" << t.latitude << ", longitude=" << t.longitude << "}";
  return o;
}

ostream &operator<<(ostream &o, const Coordinates::LatLng &c)
{
  o << "{ latitude=" << c.latitude.printable() << ", longitude: " << c.longitude.printable() << "}";
  return o;
}

ostream &operator<<(ostream &o, const Coordinates::AltAzimuth &c)
{
  o << "{ altitude=" << c.altitude.printable() << ", longitude: " << c.azimuth.printable() << "}";
  return o;
}

string Coordinates::Equatorial::toString() const
{
  stringstream o;
  o << "rightAscension=" << rightAscension.printable(Angle::Hourly) << ", declination: " << declination.printable();
  return o.str();
}

ostream &operator<<(ostream &o, const Coordinates::Equatorial &c)
{
  o << "{ " << c.toString() << " }";
  return o;
}


