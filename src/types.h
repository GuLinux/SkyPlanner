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

class Angle {
  public:
    Angle();
    static Angle degrees(double degrees);
    static Angle radians(double radians);
    static Angle hours(double hours);
    double degrees() const;
    double radians() const;
    double hours() const;
    enum Format { Hourly, Degrees };
    std::string printable(Format format = Degrees) const;
    operator bool() const;
  private:
    Angle(double degrees);
    const double _degrees;
    bool valid = false;
};

namespace Coordinates {
  struct Equatorial {
    Angle rightAscension;
    Angle declination;
    operator bool() { return rightAscension && declination; }
  };

  struct AltAzimuth {
    Angle altitude;
    Angle azimuth;
    operator bool() { return altitude && azimuth; }
  };

  struct LatLng {
    Angle latitude;
    Angle longitude;
    operator bool() { return latitude && longitude; }
  };
}
#endif


