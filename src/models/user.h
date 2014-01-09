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

#ifndef USER_H
#define USER_H
#include <Wt/Auth/Dbo/AuthInfo>

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/Types>
#include <Wt/Dbo/ptr>
#include <string>

namespace dbo = Wt::Dbo;

class User;
class Telescope;
class AstroSession;
typedef Wt::Auth::Dbo::AuthInfo<User> AuthInfo;

class User {
public:
  template<class Action>
  void persist(Action& a)
  {
    dbo::hasMany(a, _telescopes, dbo::ManyToOne);
    dbo::hasMany(a, _astroSessions, dbo::ManyToOne);
  }
  dbo::collection<dbo::ptr<Telescope>> telescopes() const;
  dbo::collection<dbo::ptr<AstroSession>> astroSessions() const;
private:
  dbo::collection<dbo::ptr<Telescope>> _telescopes;
  dbo::collection<dbo::ptr<AstroSession>> _astroSessions;
};

typedef dbo::ptr<User> UserPtr;
#endif // USER_H
