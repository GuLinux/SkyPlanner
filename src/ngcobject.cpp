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
#include <map>

using namespace std;
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
NgcObject::NebulaType NgcObject::type() const
{
  return _type;
}
/*
    enum NebulaType
    {
            NebGx=0,     //!< Galaxy
            NebOc=1,     //!< Open star cluster
            NebGc=2,     //!< Globular star cluster, usually in the Milky Way Galaxy
            NebN=3,      //!< Bright emission or reflection nebula
            NebPn=4,     //!< Planetary nebula
            NebDn=5,     //!< ??? Dark Nebula?      Does not exist in current catalog
            NebIg=6,     //!< ??? Irregular Galaxy? Does not exist in current catalog
            NebCn=7,     //!< Cluster associated with nebulosity
            NebUnknown=8 //!< Unknown type, catalog errors, "Unidentified Southern Objects" etc.
    };
*/
std::string NgcObject::typeDescription() const
{
  static map<NebulaType,string> descriptions {
    {NebGx, "Galaxy"},
    {NebOc, "Open star cluster"},
    {NebGc, "Globular star cluster"},
    {NebN, "Bright emission/reflection nebula"},
    {NebPn, "Planetary nebula"},
    {NebDn, "Dark nebula"},
    {NebIg, "Irregular galaxy"},
    {NebCn, "Cluster with nebulosity"},
    {NebUnknown, "Unknown"},
  };
  return descriptions[type()];
}

