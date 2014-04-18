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

#ifndef ASTROSESSION_H
#define ASTROSESSION_H
#include <Wt/Dbo/Dbo>
#include "types.h"

class Ephemeris;
class NgcObject;
namespace Wt {
class WDateTime;
}

class User;
class AstroSessionObject;
namespace dbo = Wt::Dbo;

class AstroSession
{
public:
  AstroSession();
  AstroSession(const std::string &name, const boost::posix_time::ptime &when, const dbo::ptr<User> &user = dbo::ptr<User>() );
  AstroSession(const std::string &name, const Wt::WDateTime &when, const dbo::ptr<User> &user = dbo::ptr<User>() );
  
  std::string name() const;
  boost::posix_time::ptime when() const;
  boost::gregorian::date date() const;
  Wt::WDateTime wDateWhen() const;
  dbo::collection<dbo::ptr<AstroSessionObject>> astroSessionObjects() const;
  Coordinates::LatLng position() const;
  dbo::ptr<User> user() const;
  void setPosition(const Coordinates::LatLng &position);
  void setName(const std::string &name);
  void setDateTime(const Wt::WDateTime &when);
  template<typename Action>
  void persist(Action& a) {
    dbo::field(a, _name, "name");
    dbo::field(a, _when, "when");
    dbo::field(a, _latitude, "latitude");
    dbo::field(a, _longitude, "longitude");
    dbo::hasMany(a, _astroSessionObjects, dbo::ManyToOne);
    dbo::belongsTo(a, _user);
  }
private:
  std::string _name;
  boost::posix_time::ptime _when;
  dbo::ptr<User> _user;
  dbo::collection<dbo::ptr<AstroSessionObject>> _astroSessionObjects;
  boost::optional<double> _latitude;
  boost::optional<double> _longitude;
};

typedef dbo::ptr<AstroSession> AstroSessionPtr;
#endif // ASTROSESSION_H
