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
#ifndef CATALOGUE_H
#define CATALOGUE_H

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/Types>
#include <Wt/Dbo/ptr>
#include <Wt/WString>
#include <string>
#include "types.h"

class NebulaDenomination;
namespace dbo = Wt::Dbo;
class Catalogue
{
public:
  enum SearchMode { ByName = 0x0, ByCatalog = 0x1, ByNameAndType = 0x2 };
  enum HiddenMode { Visible = 0x0, HiddenSearch = 0x1, HiddenEverywhere = 0xFF };
  Catalogue() = default;
  Catalogue(const std::string &name, const std::string &code, int priority, SearchMode searchMode, HiddenMode hiddenMode);
  dbo::collection<dbo::ptr<NebulaDenomination>> nebulae() const;
  std::string name() const;
  std::string code() const;
  int priority() const;
  HiddenMode hidden() const;
  SearchMode searchMode() const;
  
    template<class Action>
    void persist(Action& a) {
        dbo::field(a, _name, "name");
        dbo::field(a, _code, "code");
        dbo::field(a, _priority, "priority");
        dbo::field(a, _searchMode, "search_mode");
        dbo::field(a, _hidden, "hidden");
        dbo::hasMany(a, _nebulae, dbo::ManyToOne);
    }
private:
  std::string _name;
  std::string _code;
  HiddenMode _hidden;
  int _priority;
  SearchMode _searchMode;
  dbo::collection<dbo::ptr<NebulaDenomination>> _nebulae;
};
typedef dbo::ptr<Catalogue> CataloguePtr;

#endif // CATALOGUE_H
