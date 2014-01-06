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
#include "utils/format.h"

using namespace std;

boost::optional<string> NebulaDenomination::catalogue() const
{
  return _catalogue;
}
boost::optional<string> NebulaDenomination::comment() const
{
  return _comment;
}
string NebulaDenomination::name() const
{
  return _name;
}
Wt::Dbo::ptr< NgcObject > NebulaDenomination::ngcObject() const
{
  return _ngcObject;
}
boost::optional<std::string> NebulaDenomination::number() const
{
  return _number;
}


bool NebulaDenomination::isNgcIc() const
{
  return _catalogue && _number && (*_catalogue == "NGC" || *_catalogue == "IC");
}

string NebulaDenomination::search() const {
 switch(_searchMode) {
    case ByName:
      return name();
    case ByCatalog:
      return format("%s %s") % *_catalogue % *_number;
    case ByNameAndType:
      return format("%s %s") % _ngcObject->typeDescription().toUTF8() % name();
 }
}
