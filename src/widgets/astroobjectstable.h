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

#ifndef ASTROOBJECTSTABLE_H
#define ASTROOBJECTSTABLE_H

#include <Wt/WCompositeWidget>
#include "utils/d_ptr.h"
#include "models/Models"
#include <functional>

class Session;

class AstroObjectsTable : public Wt::WCompositeWidget
{
public:
  struct AstroObject {
    AstroSessionPtr astroSession;
    NgcObjectPtr object;
    Ephemeris::BestAltitude bestAltitude; 
  };
  struct Selection {
    Selection() = default;
    Selection(const NgcObjectPtr &object, const std::string &css, std::function<void(Wt::WTableRow*)> onSelectionFound) : object(object), css(css), onSelectionFound(onSelectionFound) {}
    NgcObjectPtr object;
    std::string css;
    std::function<void(Wt::WTableRow*)> onSelectionFound = [](Wt::WTableRow*) {};
    operator bool() const { return object && !css.empty(); }
  };
  AstroObjectsTable(Session &session, Wt::WContainerWidget *parent = 0);
  void populate(const std::vector<AstroObject> &objects, const TelescopePtr &telescope, const Timezone &timezone, const Selection &selection = {});
private:
  D_PTR;
};

#endif
