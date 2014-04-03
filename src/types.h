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

#ifndef ASTROPLANNER_TYPES_H
#define ASTROPLANNER_TYPES_H

#include <string>
#include <boost/date_time.hpp>

class Angle {
  public:
    Angle();
    struct Sexagesimal {
      int degrees;
      int minutes;
      double seconds;
    };
    struct Hours {
      int hours;
      int minutes;
      double seconds;
    };
    static Angle degrees(double degrees);
    static Angle arcMinutes(double arcMinutes);
    static Angle radians(double radians);
    static Angle hours(double hours);
    double degrees() const;
    double arcMinutes() const;
    Sexagesimal sexagesimal() const;
    Hours sexagesimalHours() const;
    double radians() const;
    double hours() const;
    enum Format { Hourly, Degrees };
    enum PrintFormat{ UTF8, HTML};
    std::string printable(Format format = Degrees, PrintFormat printFormat = UTF8 ) const;
    bool valid() const;
    Angle operator*(double coefficient) const;
    Angle &operator+=(const Angle &other) { _degrees += other.degrees(); return *this; }
    Angle &operator /=(double d) { _degrees /= d; return *this; }
    bool operator>(const Angle &other) const { return degrees() > other.degrees(); }
    bool operator<(const Angle &other) const { return degrees() < other.degrees(); }
    bool operator==(const Angle &other) const { return degrees() == other.degrees(); }
    bool operator<=(const Angle &other) const { return (*this == other) || (*this < other); }
    bool operator>=(const Angle &other) const { return (*this == other) || (*this > other); }
    bool operator!=(const Angle &other) const { return ! (*this == other); }
    Angle operator-(const Angle &other) const { return Angle::degrees(_degrees - other.degrees()); }
  private:
    Angle(double degrees);
    double _degrees;
    bool _valid = false;
};

namespace Coordinates {
  struct Equatorial {
    Angle rightAscension;
    Angle declination;
    operator bool() { return rightAscension.valid() && declination.valid(); }
    std::string toString() const;
  };

  struct AltAzimuth {
    Angle altitude;
    Angle azimuth;
    operator bool() { return altitude.valid() && azimuth.valid(); }
  };

  struct LatLng {
    Angle latitude;
    Angle longitude;
    operator bool() { return latitude.valid() && longitude.valid(); }
  };
}

struct Timezone {
  int dstOffset = 0;
  int rawOffset = 0;
  std::string timeZoneId;
  std::string timeZoneName;
  double latitude;
  double longitude;
  std::string key(const boost::posix_time::ptime &when, const std::string &language);
  static std::string key(double lat, double lng, const boost::posix_time::ptime &when, const std::string &language);
  static Timezone from(const std::string &response, double lat, double lng);
  boost::posix_time::ptime fix(const boost::posix_time::ptime &src) const;
  boost::posix_time::ptime fixUTC(const boost::posix_time::ptime &src) const;
  operator bool() const { return !timeZoneId.empty(); }
};

std::ostream &operator<<(std::ostream &o, const Timezone &t);
std::ostream &operator<<(std::ostream &o, const Coordinates::LatLng &c);
std::ostream &operator<<(std::ostream &o, const Coordinates::AltAzimuth &c);
std::ostream &operator<<(std::ostream &o, const Coordinates::Equatorial &c);

#endif


