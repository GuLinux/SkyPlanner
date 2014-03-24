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

#ifndef EPHEMERIS_CACHE_H
#define EPHEMERIS_CACHE_H
#include <Wt/Dbo/Dbo>
#include "ephemeris.h"


namespace dbo = Wt::Dbo;
class AstroSession;
class NgcObject;

class EphemerisCache
{
public:
    explicit EphemerisCache() = default;
    explicit EphemerisCache(const Ephemeris::BestAltitude &bestAltitude, const dbo::ptr<NgcObject> &ngcObject, const dbo::ptr<AstroSession> &astroSession)
    : _astroSession(astroSession), _ngcObject(ngcObject), _transitTime(bestAltitude.when), _altitude(bestAltitude.coordinates.altitude.degrees()), _azimuth(bestAltitude.coordinates.azimuth.degrees()) {}

//  explicit Telescope(const std::string &name, int diameter, int focalLength, bool isDefault = false);
  template<typename Action>
  void persist(Action& a) {
    dbo::field(a, _transitTime, "transit_time");
    dbo::field(a, _altitude, "altitude");
    dbo::field(a, _azimuth, "azimuth");
  
    dbo::belongsTo(a, _astroSession);
    dbo::belongsTo(a, _ngcObject);
  }
  Ephemeris::BestAltitude bestAltitude() const { return { {Angle::degrees(_altitude), Angle::degrees(_azimuth)},  _transitTime};  }
private:
  dbo::ptr<AstroSession> _astroSession;
  dbo::ptr<NgcObject> _ngcObject;
  boost::posix_time::ptime _transitTime;
  double _altitude;
  double _azimuth;
};


namespace Wt {
  namespace Dbo {

    template<>
    struct dbo_traits<EphemerisCache> : public dbo_default_traits {
//      static const char *surrogateIdField() { return 0; }
      static const char *versionField() { return 0; }
    };

  }
}

typedef dbo::ptr<EphemerisCache> EphemerisCachePtr;

#endif // EPHEMERIS_CACHE_H
