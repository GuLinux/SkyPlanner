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

#include "ngcobject.h"
#include "nebuladenomination.h"


NgcObject::NgcObject()
{
}

NgcObject::~NgcObject()
{
}

float NgcObject::angularSize() const
{
  return _angularSize;
}

float NgcObject::declination() const
{
  return _declination;
}

float NgcObject::magnitude() const
{
  return _magnitude;
}
Wt::Dbo::collection< Wt::Dbo::ptr< NebulaDenomination > > NgcObject::nebulae() const
{
  return _nebulae;
}
std::string NgcObject::objectId() const
{
  return _objectId;
}
float NgcObject::rightAscension() const
{
  return _rightAscension;
}
int NgcObject::type() const
{
  return _type;
}
