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

#ifndef ASTROOBJECTSTABLE_H
#define ASTROOBJECTSTABLE_H

#include <Wt/WCompositeWidget>
#include "c++/dptr.h"
#include "models/Models"
#include <functional>
#include <boost/logic/tribool.hpp>
#include "widgets/filters/filterbyrangewidget.h"

class Session;
class ObjectPopupMenu;

typedef FilterByRangeWidget<double> FilterByMagnitudeWidget;
typedef FilterByRangeWidget<Angle> FilterByAltitudeWidget;
typedef FilterByRangeWidget<Angle> FilterByAngularSizeWidget;
typedef FilterByRangeWidget<boost::posix_time::ptime> FilterByTimeWidget;

class AstroObjectsTable : public Wt::WCompositeWidget
{
public:
  struct AstroObject {
    AstroSessionPtr astroSession;
    NgcObjectPtr object;
    Ephemeris::BestAltitude bestAltitude;
    std::string rowStyle;
    boost::optional<Ephemeris::Planet> planet;

    // Display widgets
    Wt::WWidget *names(Session &session, ObjectPopupMenu *popupMenu, std::function<void(Wt::WMouseEvent)> onClick = [](Wt::WMouseEvent){} ) const;
    Wt::WString typeDescription() const;
    Angle ar() const;
    Angle dec() const;
    ConstellationFinder::Constellation constellation() const;    
    Angle angularSize() const;
    double magnitude() const;
    DateTime transitTime() const;
    Ephemeris::RiseTransitSet rts() const;
    Angle maxAltitude() const;
    Wt::WWidget *difficultyWidget(const TelescopePtr &telescope) const;
  };
  struct Row {
    AstroObject astroObject;
    Wt::WTableRow *tableRow;
    std::function<void()> toggleMoreInfo;
    std::map<std::string, Wt::WWidget*> actions;
  };
  struct Selection {
    typedef std::function<void(Row)> OnSelectionFound;
    Selection(const NgcObjectPtr &object = {}, const std::string &css = {}, OnSelectionFound onSelectionFound = [](Row){}) : object(object), css(css), onSelectionFound(onSelectionFound) {}
    NgcObjectPtr object;
    std::string css;
    OnSelectionFound onSelectionFound;
    operator bool() const { return object && !css.empty(); }
  };
  struct Action {
    Action() = default;
    Action(const std::string &name, std::function<void(Row, Wt::WWidget*)> onClick, const std::string buttonCss = std::string{}) : name(name), onClick(onClick), buttonCss(buttonCss) {}
    std::string name;
    std::function<void(Row, WWidget*)> onClick;
    std::string buttonCss;
    std::function<void(Wt::WMenuItem *, Row)> onMenuItemCreated = [](Wt::WMenuItem*, Row) {};
    std::function<void(Wt::WPushButton *, Row)>  onButtonCreated = [](Wt::WPushButton*, Row) {};
  };
  struct Filters {
    std::set<NgcObject::NebulaType> types;
    double minimumMagnitude;
    double maximumMagnitude;
    ConstellationFinder::Constellation constellation;
    CataloguePtr catalogue;
    Angle minimumAltitude;
    Angle maximumAltitude = Angle::degrees(90);
    boost::posix_time::ptime start_time;
    boost::posix_time::ptime end_time;
    boost::logic::tribool observed = boost::logic::indeterminate;
    Angle minimumAngularSize;
    Angle maximumAngularSize;
  };
  struct Page {
    typedef std::function<void(long)> Change;
    Page(long current = -1, long total = -1, std::size_t pageSize = 15, Change change = [](long){}) : current(current), total(total), pageSize(pageSize), change(change) {}
    long current;
    long total;
    std::size_t pageSize;
    std::function<void(long)> change;
    operator bool() const;
    static Page fromCount(long pageNumber, long count, std::function<void(long)> onChange, std::size_t pageSize = 15);
    long offset() const;
  };
  enum Column { Names, Type, AR, DEC, Constellation, AngularSize, Magnitude, ObservationTime, MaxAltitude, Difficulty, RiseTime, TransitTime, SetsTime };
  enum FiltersButtonDisplay { NoFiltersButton, FiltersButtonIntegrated, FiltersButtonExternal };
  static const std::list<Column> allColumns;
  AstroObjectsTable(Session &session,
		    const std::vector<Action> &actions = {},
		    FiltersButtonDisplay showFilters = FiltersButtonIntegrated,
                    const std::set<NgcObject::NebulaType> &initialTypes = NgcObject::allNebulaTypes(),
                    const std::list<AstroObjectsTable::Column> &columns = allColumns,
                    Wt::WContainerWidget *parent = 0);
  void populate(const std::vector<AstroObject> &objects, const TelescopePtr &telescope, const Timezone &timezone, const Page &page = {}, const Selection &selection = {});
  void clear();
  Wt::Signal<Filters> &filtersChanged() const;
  Filters currentFilters() const;
  void setMagnitudeRange(const FilterByRange::Range<double> &magnitudeRange);
  void setTimeRange(const FilterByRange::Range<boost::posix_time::ptime> &timeRange, const Timezone &timezone);
  Wt::WContainerWidget *tableFooter() const;
  void setTableAttribute(const std::string &attributeName, const std::string &attributeValue);
  void setResponsive(bool responsive);
  Wt::Signal<AstroSessionObjectPtr> &objectsListChanged() const;
  void forceActionsAsToolBar(bool force);
  void planets(const AstroSessionPtr &astroSession, const Timezone &timezone);
  Wt::WPushButton *filtersButton() const;
  std::vector<Row> rows() const;
  static std::list<Column> PlanetColumns();
private:
  D_PTR;
};

std::ostream &operator<<(std::ostream &o, const AstroObjectsTable::Filters &f);

#endif
