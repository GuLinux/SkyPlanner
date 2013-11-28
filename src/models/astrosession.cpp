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

AstroSession::AstroSession(const std::string& name, const boost::posix_time::ptime& when)
  : _name(name), _when(when)
{
}
AstroSession::AstroSession(const std::string& name, const WDateTime& when)
  : _name(name), _when(when.toPosixTime())
{
}

std::string AstroSession::name() const
{
  return _name;
}
Wt::WDateTime AstroSession::wDateWhen() const
{
  return WDateTime::fromPosixTime(when());
}
boost::posix_time::ptime AstroSession::when() const
{
  return _when;
}

Dbo::collection< Dbo::ptr< AstroSessionObject > > AstroSession::astroSessionObjects() const
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

AstroSession::ObservabilityRange AstroSession::observabilityRange( const Ephemeris &ephemeris ) const
{
  return {  
    ephemeris.sun( when() ).set,
    ephemeris.sun( when() + boost::posix_time::time_duration{24 ,0,0}).rise
  };
}


AstroSession::ObservabilityRange &AstroSession::ObservabilityRange::delta( const boost::posix_time::time_duration &duration )
{
  begin += duration;
  end -= duration;
  return *this;
}
