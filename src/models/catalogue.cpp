/*
 * Copyright (C) 2014  Marco Gulino <marco.gulino@gmail.com>
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

using namespace Wt;
using namespace std;

Catalogue::Catalogue(const std::string &name, const std::string &code, int priority, SearchMode searchMode, HiddenMode hiddenMode)
  : _name(name), _code(code), _priority(priority), _searchMode(searchMode), _hidden(hiddenMode)
{
}

Dbo::collection< NebulaDenominationPtr> Catalogue::nebulae() const
{
  return _nebulae;
}

Catalogue::HiddenMode Catalogue::hidden() const
{
  return _hidden;
}


string Catalogue::code() const
{
  return _code;
}

string Catalogue::name() const
{
  return _name;
}

int Catalogue::priority() const
{
  return _priority;
}

Catalogue::SearchMode Catalogue::searchMode() const
{
  return _searchMode;
}


