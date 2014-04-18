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

#include "user.h"
#include "Models"
using namespace Wt;
Dbo::collection<Dbo::ptr<Telescope>> User::telescopes() const
{
  return _telescopes;
}

Dbo::collection< Dbo::ptr< AstroSession > > User::astroSessions() const
{
  return _astroSessions;
}

bool User::isAdmin() const
{
  return _isAdmin;
}

Dbo::weak_ptr<AuthInfo> User::authInfo() const
{
  return _authInfo;
}

WString User::loginName() const
{
  return _authInfo->identity("loginname");
}
