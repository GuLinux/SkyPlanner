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

#ifndef ASTROSESSIONOBJECT_H
#define ASTROSESSIONOBJECT_H
#include <Wt/Dbo/Dbo>
#include "types.h"
#include "ephemeris.h"

class Telescope;
class NgcObject;
class AstroSession;
namespace dbo = Wt::Dbo;
class AstroSessionObject
{
public:
  AstroSessionObject();
  AstroSessionObject(const dbo::ptr<NgcObject> &ngcObject);
  dbo::ptr<NgcObject> ngcObject() const;
  AstroSessionPtr astroSession() const;
  std::string description() const;
  boost::optional<std::string> report() const;
  void setDescription(const std::string &description);
  void setReport(const std::string &report);
  bool observed() const;
  void setObserved(bool observed = true);
  
  template<typename Action>
  void persist(Action& a) {
    dbo::field(a, _description, "description");
    dbo::field(a, _report, "report");
    dbo::field(a, _observed, "observed");
    dbo::field(a, _transitTime, "transit_time");
    dbo::field(a, _altitude, "altitude");
    dbo::field(a, _azimuth, "azimuth");
    dbo::field(a, _ephemeris_context_key, "ephemeris_context_key");
    dbo::belongsTo(a, _astroSession);
    dbo::belongsTo(a, _ngcObject);
  }
  Coordinates::Equatorial coordinates() const;
  Ephemeris::BestAltitude bestAltitude(const Ephemeris &ephemeris, const Timezone &timezone) const;
  static Ephemeris::BestAltitude bestAltitude(const AstroSessionPtr &astroSession, const dbo::ptr<NgcObject> &ngcObject, const Ephemeris &ephemeris);
  /** It's a simil-percentage evaluation:
    if we have a valid telescope here, and this object magnitude doesn't exceed its magnitude
    limit, we return the difficulty percentage (0-100).
    If it does exceed the magnitude limit (with a tolerance delta), then the percentage can exceed 100.
    If we don't have a telescope, we simply return -1.
  **/
  int32_t difficulty(const dbo::ptr<Telescope> &telescope) const;
  static void generateEphemeris(const Ephemeris &ephemeris, const AstroSessionPtr &astroSession, const Timezone &timezone, dbo::Transaction &transaction);
private:
  AstroSessionPtr _astroSession;
  dbo::ptr<NgcObject> _ngcObject;
  std::string _description;
  bool _observed = false;
  boost::optional<double> _altitude, _azimuth;
  boost::optional<boost::posix_time::ptime> _transitTime;
  boost::optional<std::string> _ephemeris_context_key;
  boost::optional<std::string> _report;
};

typedef AstroSessionObjectPtr AstroSessionObjectPtr;
#endif // ASTROSESSIONOBJECT_H
