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
  struct Row {
    AstroObject astroObject;
    Wt::WTableRow *tableRow;
    std::function<void()> toggleMoreInfo;
  };
  struct Selection {
    Selection() = default;
    Selection(const NgcObjectPtr &object, const std::string &css, std::function<void(Row)> onSelectionFound) : object(object), css(css), onSelectionFound(onSelectionFound) {}
    NgcObjectPtr object;
    std::string css;
    std::function<void(Row)> onSelectionFound = [](Row) {};
    operator bool() const { return object && !css.empty(); }
  };
  struct Action {
    Action() = default;
    Action(const std::string &name, std::function<void(Row)> onClick, const std::string buttonCss = std::string{}) : name(name), onClick(onClick), buttonCss(buttonCss) {}
    std::string name;
    std::function<void(Row)> onClick;
    std::string buttonCss;
    std::function<void(Wt::WMenuItem *, Row)> onMenuItemCreated = [](Wt::WMenuItem*, Row) {};
  };
  struct Filters {
    std::set<NgcObject::NebulaType> types;
    double minimumMagnitude = -200;
    ConstellationFinder::Constellation constellation;
    CataloguePtr catalogue;
  };
  struct Page {
    long current = -1;
    long total = -1;
    std::size_t pageSize = 15;
    std::function<void(int)> change;
    operator bool() const;
  };
  AstroObjectsTable(Session &session, const std::vector<Action> &actions = {}, bool showFilters = true, const std::set<NgcObject::NebulaType> &initialTypes = NgcObject::allNebulaTypes(), Wt::WContainerWidget *parent = 0);
  void populate(const std::vector<AstroObject> &objects, const TelescopePtr &telescope, const Timezone &timezone, const Selection &selection = {}, const Page &page = {});
  void clear();
  Wt::Signal<Filters> &filtersChanged() const;
  Filters currentFilters() const;
  void setMaximumMagnitude(double magnitudeLimit);
  Wt::WContainerWidget *tableFooter() const;
private:
  D_PTR;
};


#endif