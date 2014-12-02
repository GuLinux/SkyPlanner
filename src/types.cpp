/*
 * Copyright (C) 2014  Marco Gulino <marco.gulino@gmail.com>
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
  string signFix = degrees() < 0 ? "-" : "";
  if(format == Hourly) {
    auto sHours = sexagesimalHours();
    return (boost::format("%s%dh %2dm %.1fs") % ( sHours.hours == 0 ? signFix : "" ) % sHours.hours % sHours.minutes % sHours.seconds ).str();

  }
  
  Sexagesimal asSexagesimal = sexagesimal();
  auto sign = asSexagesimal.degrees == 0 ? signFix : "";
  auto deg_symbol = printFormat == HTML ? "&deg;" : "\302\260";
  if(format == IntDegrees) {
    return (boost::format("%s%d%s") % sign % asSexagesimal.degrees % deg_symbol).str();
  }

  return (boost::format("%s%d%s %2d' %.1f\"") % sign % asSexagesimal.degrees % deg_symbol % asSexagesimal.minutes % asSexagesimal.seconds ).str();
}

 
boost::posix_time::time_duration Timezone::offset() const
{
  return boost::posix_time::seconds(dstOffset) + boost::posix_time::seconds(rawOffset);
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


DateTime DateTime::fromUTC(const boost::posix_time::ptime &utc, const Timezone &timezone)
{
  return {utc, utc + timezone.offset(), timezone};
}

DateTime DateTime::fromLocal(const boost::posix_time::ptime &local, const Timezone &timezone)
{
  return {local - timezone.offset(), local, timezone};
}

#ifndef TESTS_NO_WT
#include <Wt/WApplication>
#include <Wt/WDateTime>
#endif

string DateTime::str(DateTime::PrintFormat format, DateTime::TZone tzone) const
{
  static map<PrintFormat, function<string(boost::posix_time::ptime)>> formats {
    {HoursAndMinutes, [this](const boost::posix_time::ptime &t){ return (::format("%02d:%02d") % t.time_of_day().hours() % t.time_of_day().minutes()).str(); }},
    {HoursMinutesSeconds, [this](const boost::posix_time::ptime &t){ return (::format("%02d:%02d:%02d") % t.time_of_day().hours() % t.time_of_day().minutes() % t.time_of_day().seconds() ).str(); }},
#ifdef TESTS_NO_WT
    {DateShort, [this](const boost::posix_time::ptime &t){return (::format("%d %s, %02d:%02d") % t.date().day() % t.date().month().as_short_string() % t.time_of_day().hours() % t.time_of_day().minutes()).str(); }},
#else
    {DateShort, [this](const boost::posix_time::ptime &t){return (WDateTime::fromPosixTime(t).toString("d MMM hh:mm")).toUTF8(); }},
#endif
  };

  return formats[format](UTC ? utc : localtime);
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


