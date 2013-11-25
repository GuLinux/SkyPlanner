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
  AstroSession(const std::string &name, const boost::posix_time::ptime &when);
  AstroSession(const std::string &name, const Wt::WDateTime &when);
  
  std::string name() const;
  boost::posix_time::ptime when() const;
  Wt::WDateTime wDateWhen() const;
  dbo::collection<dbo::ptr<AstroSessionObject>> astroSessionObjects();
  template<typename Action>
  void persist(Action& a) {
    dbo::field(a, _name, "name");
    dbo::field(a, _when, "when");
    dbo::hasMany(a, _astroSessionObjects, dbo::ManyToOne);
    dbo::belongsTo(a, _user);
  }
private:
  std::string _name;
  boost::posix_time::ptime _when;
  dbo::ptr<User> _user;
  dbo::collection<dbo::ptr<AstroSessionObject>> _astroSessionObjects;
};

#endif // ASTROSESSION_H
