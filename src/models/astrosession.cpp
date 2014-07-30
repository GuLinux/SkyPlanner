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

#include "Models"
#include "astrosession.h"
#include <Wt/WDateTime>

using namespace Wt;
using namespace std;

AstroSession::AstroSession()
{
}

AstroSession::AstroSession(const std::string& name, const boost::posix_time::ptime& when, const dbo::ptr<User> &user)
  : _name(name), _when(when), _user(user)
{
}
AstroSession::AstroSession(const std::string& name, const WDateTime& when, const dbo::ptr<User> &user)
  : _name(name), _when(when.toPosixTime()), _user(user)
{
}

void AstroSession::setDateTime(const Wt::WDateTime &when)
{
  _when = when.toPosixTime();
}

void AstroSession::setName(const std::string &name)
{
  _name = name;
}

std::string AstroSession::name() const
{
  return _name;
}
Wt::WDateTime AstroSession::wDateWhen() const
{
  return WDateTime::fromPosixTime(when());
}

boost::gregorian::date AstroSession::date() const
{
  return _when.date();
}


boost::posix_time::ptime AstroSession::when() const
{
  return _when;
}
  
Dbo::ptr<User> AstroSession::user() const
{
  return _user;
}

Dbo::collection< AstroSessionObjectPtr > AstroSession::astroSessionObjects() const
{
  return _astroSessionObjects;
}

Coordinates::LatLng AstroSession::position() const
{
  if(!_latitude || !_longitude) {
    return Coordinates::LatLng{Angle(), Angle()};
  }
  return Coordinates::LatLng{Angle::degrees(*_latitude), Angle::degrees(*_longitude)};
}

void AstroSession::setPosition(const Coordinates::LatLng& position)
{
  _latitude.reset(position.latitude.degrees());
  _longitude.reset(position.longitude.degrees());
}

void AstroSession::setReport(const std::string &report)
{
  if(report.empty())
    _report.reset();
  _report.reset(report);
}

boost::optional<std::string> AstroSession::report() const
{
  return _report;
}
