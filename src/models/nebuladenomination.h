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
  std::string catalogue() const;
  int number() const;
  std::string name() const;
  std::string comment() const;
  dbo::ptr<NgcObject> ngcObject() const;
  template<class Action>
  void persist(Action& a) {
    dbo::field(a, _catalogue, "catalogue");
    dbo::field(a, _number, "number");
    dbo::id(a, _name, "name");
    dbo::field(a, _comment, "comment");
    dbo::belongsTo(a, _ngcObject);
  }
private:
  std::string _catalogue;
  int _number;
  std::string _name;
  std::string _comment;
  dbo::ptr<NgcObject> _ngcObject;
};


namespace Wt {
    namespace Dbo {

        template<>
        struct dbo_traits<NebulaDenomination> : public dbo_default_traits {
	    typedef std::string IdType;
            static IdType invalidId() {
                return std::string();
            }
            static const char *surrogateIdField() {
	      return 0;
	    }
            static const char *versionField() {
              return 0;
            }
        };

    }
}


#endif // NEBULADENOMINATION_H