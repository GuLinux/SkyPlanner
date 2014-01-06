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

#ifndef NEBULADENOMINATION_H
#define NEBULADENOMINATION_H

#include <Wt/Dbo/Dbo>
#include <string>

class NgcObject;
namespace dbo = Wt::Dbo;

class NebulaDenomination
{
public:
  enum SearchMode { ByName = 0x0, ByCatalog = 0x1, ByNameAndType = 0x2 };
  boost::optional<std::string> catalogue() const;
  boost::optional<std::string> number() const;
  std::string name() const;
  boost::optional<std::string> comment() const;
  dbo::ptr<NgcObject> ngcObject() const;
  template<class Action>
  void persist(Action& a) {
    dbo::field(a, _catalogue, "catalogue");
    dbo::field(a, _number, "number");
    dbo::field(a, _name, "name");
    dbo::field(a, _comment, "comment");
    dbo::field(a, _searchMode, "search_mode");
    dbo::belongsTo(a, _ngcObject);
  }

  bool isNgcIc() const;
  std::string search() const;

private:
  boost::optional<std::string> _catalogue;
  boost::optional<std::string> _number;
  std::string _name;
  boost::optional<std::string> _comment;
  dbo::ptr<NgcObject> _ngcObject;
  SearchMode _searchMode;
};

typedef dbo::ptr<NebulaDenomination> NebulaDenominationPtr;

namespace Wt {
    namespace Dbo {

        template<>
        struct dbo_traits<NebulaDenomination> : public dbo_default_traits {
            static const char *versionField() {
              return 0;
            }
        };

    }
}


#endif // NEBULADENOMINATION_H
