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

class NgcObject;
class AstroSession;
namespace dbo = Wt::Dbo;
class AstroSessionObject
{
public:
  AstroSessionObject();
  AstroSessionObject(const dbo::ptr<NgcObject> &ngcObject);
  dbo::ptr<NgcObject> ngcObject() const;
  template<typename Action>
  void persist(Action& a) {
    dbo::belongsTo(a, _astroSession);
    dbo::belongsTo(a, _ngcObject);
  }
  Coordinates::Equatorial coordinates() const;
  Ephemeris::BestAltitude bestAltitude(const Ephemeris &ephemeris, int rangeDeltaInHours = 0) const;
private:
  dbo::ptr<AstroSession> _astroSession;
  dbo::ptr<NgcObject> _ngcObject;
};

#endif // ASTROSESSIONOBJECT_H
