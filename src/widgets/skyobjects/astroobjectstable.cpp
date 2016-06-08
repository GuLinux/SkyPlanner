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

#include "astroobjectstable.h"
#include "private/astroobjectstable_p.h"
#include <Wt/WContainerWidget>
#include <Wt/WText>
#include <Wt/WPushButton>
#include <Wt/Utils>
#include <Wt/WPopupMenu>
#include <Wt/WComboBox>
#include <Wt/WLabel>
#include <Wt/WToolBar>
#include <wt_helpers.h>
#include "models/Models"
#include "astroobjectwidget.h"
#include "objectnameswidget.h"
#include "utils/format.h"
#include "objectdifficultywidget.h"
#include "objectpopupmenu.h"

using namespace Wt;
using namespace WtCommons;
using namespace std;

AstroObjectsTable::Private::Private( Session &session, const vector< AstroObjectsTable::Action > &actions, const list< AstroObjectsTable::Column > &columns, AstroObjectsTable *q )
 : session(session), actions(actions), columns(columns), q(q)
{
}

#define BY_TYPE "filter_by_type_menu"
#define BY_MAGNITUDE "filter_by_magnitude_menu"
#define BY_CONSTELLATION "filter_by_constellation_menu"
#define BY_CATALOGUE "filter_by_catalogue_menu"
#define BY_ALTITUDE "filter_by_altitude_menu"
#define BY_OBSERVED "filter_by_observed_in_any_session"
#define BY_TIME "filter_by_time"
#define BY_ANGULAR_SIZE "filter_by_angular_size"

AstroObjectsTable::AstroObjectsTable(Session &session,
				     const vector<Action> &actions,
				     FiltersButtonDisplay showFilters,
				     const set<NgcObject::NebulaType> &initialTypes,
				     const std::list<Column> &columns, WContainerWidget *parent)
  : WCompositeWidget(parent), dptr(session, actions, columns, this)
{
  d->objectsTable = WW<WTable>().addCss("table table-hover table-condensed astroobjects-table");
  d->objectsTable->setHeaderCount(1);
  WContainerWidget *container = WW<WContainerWidget>();
  if(showFilters != NoFiltersButton) {
    d->filtersBar = WW<WContainerWidget>().css("hidden-print form-inline").setInline(false);
    d->availableFilters = WW<WPopupMenu>();
    d->filtersButton = WW<WPushButton>(WString::tr("filters")).css("btn-sm hidden-print").onClick([=](const WMouseEvent &e) { d->availableFilters->popup(e); });
    if(showFilters == FiltersButtonIntegrated)
      d->filtersBar->addWidget(d->filtersButton);
    
    d->timeFilterTraits = make_shared<FilterByRange::Traits< boost::posix_time::ptime >>(boost::posix_time::second_clock::local_time());
    
    d->addFilterItem(BY_TYPE, new FilterByTypeWidget(initialTypes));
    d->addFilterItem(BY_CONSTELLATION, new FilterByConstellation);
    d->addFilterItem(BY_CATALOGUE, new FilterByCatalogue(session));
    d->addFilterItem(BY_OBSERVED, new FilterByObservedWidget{WString::tr("observed-in-any-session")});
    d->addFilterItem(BY_MAGNITUDE, new FilterByRangeWidget<double>{ {-5, 25}, {"magnitude_label", "magnitude_title", "minimum_magnitude_label", "maximum_magnitude_label"},  });
    d->addFilterItem(BY_ALTITUDE, new FilterByAltitudeWidget{ {Angle::degrees(0), Angle::degrees(90)}, {"range_angle_label", "range_angle_title", "minimum-altitude", "maximum-altitude"}, 
      make_shared<FilterByAltitudeWidget::Traits>([](const Angle &angle){ return angle.printable(Angle::IntDegrees); }) });
    d->addFilterItem(BY_ANGULAR_SIZE, new FilterByRangeWidget<Angle>{ {Angle::degrees(0), Angle::degrees(2)}, {"range_angular_size_label", "range_angular_size_title", "minimum_angular_size", "maximum_angular_size"},
      make_shared<FilterByAltitudeWidget::Traits>([](const Angle &angle){ if(angle == Angle::degrees(0) || angle == Angle::degrees(2)) return string{"N/A"}; return angle.printable(); })});
    d->addFilterItem(BY_TIME, new FilterByTimeWidget{ {}, {"range_transit_label", "range_transit_title", "range_transit_minimum", "range_transit_maximum"}, d->timeFilterTraits });

    d->availableFilters->addSeparator();
    d->availableFilters->addItem(WString::tr("reset-filters"))->triggered().connect([=](WMenuItem*, _n5) {
      for(auto item: d->filterViews)
	item.second.remove();
    });
    container->addWidget(d->filtersBar);
  }
  d->tableContainer = WW<WContainerWidget>()/*.addCss("table-responsive")*/.add(d->objectsTable).add(d->tableFooter = WW<WContainerWidget>() );
  container->addWidget( d->tableContainer );
  setImplementation(container);
}

WPushButton *AstroObjectsTable::filtersButton() const
{
  return d->filtersButton;
}

template<typename T> void AstroObjectsTable::Private::addFilterItem(const string &text, T *filterWidget)
{
  WMenuItem *item = availableFilters->addItem(WString::tr(text));
  filterWidget->changed().connect([=](_n6){ filtersChanged.emit(filters()); });
  filterViews[text] = {text, item, filterWidget, [=] {
    if(!filterViews[text].container)
      return;
    filterViews[text].container->removeWidget(filterWidget);
    delete filterViews[text].container;
    filterViews[text].container = nullptr;
    availableFilters->setItemHidden(item, false);
    filterWidget->resetDefaultValue();
  }};
  item->triggered().connect([=](WMenuItem *i, _n5) {
    availableFilters->setItemHidden(item, true);
    filterViews[text].container = WW<WContainerWidget>().add(filterWidget).setInline(true).css("filter-container badge hidden-print");
    WPushButton *closeButton = WW<WPushButton>().setTextAndFormat("&times;", XHTMLUnsafeText).css("close close-inline").onClick([=](WMouseEvent){ filterViews[text].remove(); });
    filterViews[text].container->addWidget(WW<WContainerWidget>().setInline(true).add(closeButton));
    filtersBar->addWidget(filterViews[text].container);
  });
}

AstroObjectsTable::Filters AstroObjectsTable::Private::filters() const
{
  Filters _filters;
  _filters.minimumMagnitude = filter<FilterByMagnitudeWidget>(BY_MAGNITUDE)->value().lower;
  _filters.maximumMagnitude = filter<FilterByMagnitudeWidget>(BY_MAGNITUDE)->value().upper;
  _filters.catalogue = filter<FilterByCatalogue>(BY_CATALOGUE)->selectedCatalogue();
  _filters.constellation = filter<FilterByConstellation>(BY_CONSTELLATION)->selectedConstellation();
  _filters.types = filter<FilterByTypeWidget>(BY_TYPE)->selected();
  _filters.minimumAltitude = filter<FilterByAltitudeWidget>(BY_ALTITUDE)->value().lower;
  _filters.maximumAltitude = filter<FilterByAltitudeWidget>(BY_ALTITUDE)->value().upper;
  _filters.observed = filter<FilterByObservedWidget>(BY_OBSERVED)->value();
  _filters.start_time = filter<FilterByTimeWidget>(BY_TIME)->value().lower;
  _filters.end_time = filter<FilterByTimeWidget>(BY_TIME)->value().upper;
  FilterByAngularSizeWidget::Range angularSizeRange = filter<FilterByAngularSizeWidget>(BY_ANGULAR_SIZE)->value();
  _filters.minimumAngularSize = angularSizeRange.lower == Angle::degrees(0) ? Angle() : angularSizeRange.lower;
  _filters.maximumAngularSize = angularSizeRange.upper == Angle::degrees(2) ? Angle() : angularSizeRange.upper;
  return _filters;
}

void AstroObjectsTable::setMagnitudeRange(const FilterByMagnitudeWidget::Range& magnitudeRange)
{
  d->filter<FilterByMagnitudeWidget>(BY_MAGNITUDE)->setValue(magnitudeRange);
}

void AstroObjectsTable::setTimeRange(const FilterByTimeWidget::Range &timeRange, const Timezone &timezone)
{
  d->timeFilterTraits->_start = timeRange.lower;
  d->timeFilterTraits->_formatter = [=](const boost::posix_time::ptime &t){
    return DateTime::fromUTC(t, timezone).str();
  };
  d->filter<FilterByTimeWidget>(BY_TIME)->setOuterRange(timeRange);
  d->filter<FilterByTimeWidget>(BY_TIME)->setValue(timeRange);
}


template<typename T> T* AstroObjectsTable::Private::filter(const string &name) const
{
  return reinterpret_cast<T*>(filterViews.at(name).filter);
}

AstroObjectsTable::Filters AstroObjectsTable::currentFilters() const
{
  return d->filters();
}

Signal< AstroObjectsTable::Filters > &AstroObjectsTable::filtersChanged() const
{
  return d->filtersChanged;
}


AstroObjectsTable::Page::operator bool() const
{
  return current >= 0 && total >= 0;
}




void AstroObjectsTable::clear()
{
  d->objectsTable->clear();
}

const list<AstroObjectsTable::Column> AstroObjectsTable::allColumns = {
  Names, Type, AR, DEC, Constellation, AngularSize, Magnitude, ObservationTime, MaxAltitude, Difficulty
};

void AstroObjectsTable::Private::header()
{
  struct ColumnName {
    string key;
    string abbrevKey;
    WString name() const { return WString::tr(key); }
    WString shortName() const { return abbrevKey.empty() ? name() : WString::tr(abbrevKey); }
  };
  static map<Column, ColumnName> columnKey {
    {Names, {"object_column_names"}},
    {Type, {"object_column_type"}},
    {AR, {"object_column_ar"}},
    {DEC, {"object_column_dec"}},
    {Constellation, {"object_column_constellation", "object_column_constellation_short"}},
    {AngularSize, {"object_column_angular_size", "object_column_angular_size_short"}},
    {Magnitude, {"object_column_magnitude", "object_column_magnitude_short"}},
    {RiseTime, {"object_column_rise_time", "object_column_rise_time_short"}},
    {TransitTime, {"object_column_transit_time", "object_column_transit_time_short"}},
    {SetsTime, {"object_column_set_time", "object_column_set_time_short"}},
    {ObservationTime, {"object_column_highest_time", "object_column_highest_time_short"}},
    {MaxAltitude, {"object_column_max_altitude", "object_column_max_altitude_short"}},
    {Difficulty, {"object_column_difficulty"}},
  };
  q->clear();
  int index = 0;
  for(auto column: columns) {
    objectsTable->elementAt(0,index)->addWidget(WW<WText>(columnKey[column].name()).addCss("hidden-print") );
    objectsTable->elementAt(0,index++)->addWidget(WW<WText>(columnKey[column].shortName()).addCss("visible-print printable-small-text") );
  }
}


WWidget *AstroObjectsTable::AstroObject::names(Session &session, ObjectPopupMenu *popupMenu, function<void(WMouseEvent)> onClick) const
{
  if(planet)
    return new WText{WString::tr(format("planet_%s") % planet->name) };
  
  return WW<ObjectNamesWidget>(new ObjectNamesWidget{object, session, popupMenu}).setInline(true).onClick(onClick);
}

WString AstroObjectsTable::AstroObject::typeDescription() const
{
  if(planet)
    return WString::tr("planet_type");
  return object->typeDescription(); 
}

Angle AstroObjectsTable::AstroObject::ar() const
{
  if(planet)
    return planet->coordinates.rightAscension;
  return object->coordinates().rightAscension;
}

Angle AstroObjectsTable::AstroObject::dec() const
{
  if(planet)
    return planet->coordinates.declination;
  return object->coordinates().declination;
}

ConstellationFinder::Constellation AstroObjectsTable::AstroObject::constellation() const
{
  if(planet)
    return ConstellationFinder::getName(planet->coordinates);
  return object->constellation();
}

Angle AstroObjectsTable::AstroObject::angularSize() const
{
  if(planet)
    return planet->diameter;
  return Angle::degrees(object->angularSize());
}

double AstroObjectsTable::AstroObject::magnitude() const
{
  if(planet)
    return planet->magnitude;
  return object->magnitude();
}

DateTime AstroObjectsTable::AstroObject::transitTime() const
{
  if(planet)
    return planet->rst.transit;
  return bestAltitude.when;
}

Angle AstroObjectsTable::AstroObject::maxAltitude() const
{
  if(planet)
    return planet->maxAltitude;
  return bestAltitude.coordinates.altitude;
}

Ephemeris::RiseTransitSet AstroObjectsTable::AstroObject::rts() const
{
  if(planet) {
    return planet->rst;
  }
  return bestAltitude.rst;
}


WWidget *AstroObjectsTable::AstroObject::difficultyWidget(const TelescopePtr &telescope) const
{
  return new ObjectDifficultyWidget{object, telescope, bestAltitude.coordinates.altitude.degrees() };
}

void AstroObjectsTable::setTableAttribute(const string &attributeName, const string &attributeValue)
{
  d->objectsTable->setAttributeValue(attributeName, attributeValue);
}

void AstroObjectsTable::setResponsive(bool responsive)
{
  d->tableContainer->toggleStyleClass("table-responsive", responsive);
}

void AstroObjectsTable::planets(const AstroSessionPtr &astroSession, const Timezone &timezone)
{
  Ephemeris ephemeris({astroSession->position().latitude, astroSession->position().longitude}, timezone);
  vector<AstroObjectsTable::AstroObject> _planets;
  for(auto planet: Ephemeris::allPlanets) {
    AstroObjectsTable::AstroObject astroObject;
    astroObject.planet = ephemeris.planet(planet, DateTime::fromLocal(astroSession->when(), timezone));
    _planets.push_back(astroObject);
  }
  populate(_planets, {}, timezone);
}

list< AstroObjectsTable::Column > AstroObjectsTable::PlanetColumns()
{
  return {
    AstroObjectsTable::Names,
    AstroObjectsTable::AR,
    AstroObjectsTable::DEC,
    AstroObjectsTable::Constellation, 
    AstroObjectsTable::Magnitude, 
    AstroObjectsTable::AngularSize, 
    AstroObjectsTable::RiseTime,
    AstroObjectsTable::TransitTime, 
    AstroObjectsTable::SetsTime, 
    AstroObjectsTable::MaxAltitude
  };
}


void AstroObjectsTable::populate(const vector<AstroObject> &objects, const TelescopePtr &telescope, const Timezone &timezone, const Page &page, const Selection &selection)
{
  auto clearSelection = [=] {
    d->selectedRow = nullptr;
  };
  clearSelection();
  d->rows.clear();
  d->header();
  d->tableFooter->clear();
  WTableRow *objectAddedRow = nullptr;
  for(auto astroObject: objects) {
    WTableRow *row = d->objectsTable->insertRow(d->objectsTable->rowCount());
    row->addStyleClass(astroObject.rowStyle);
    WTableRow *astroObjectRow = d->objectsTable->insertRow(d->objectsTable->rowCount());
    WTableCell *astroObjectCell = astroObjectRow->elementAt(0);
    astroObjectCell->setHidden(true);
    Row objectRow{astroObject, row};
    if(astroObject.object) {
      WPushButton *toggleMoreInfo = WW<WPushButton>(row->elementAt(0)).css("btn btn-xs pull-right hidden-print").setTextFormat(XHTMLUnsafeText).setText("&#x25bc;").setAttribute("title", WString::tr("astroobject_extended_info_title").toUTF8() );

      objectRow.toggleMoreInfo = [=] {
	toggleMoreInfo->setText(!astroObjectCell->isVisible() ? "&#x25b2;" : "&#x25bc;");
	toggleMoreInfo->toggleStyleClass("active", !astroObjectCell->isVisible());
	if(astroObjectCell->isVisible()) {
	  astroObjectCell->clear();
	  astroObjectCell->setHidden(true);
	  return;
	}
	astroObjectCell->setHidden(false);
	astroObjectCell->clear();
	astroObjectCell->addWidget(new AstroObjectWidget({astroObject.astroSession, astroObject.object, telescope, timezone}, d->session, {}, {WW<WPushButton>(WString::tr("buttons_close")).css("btn-xs").onClick([=](WMouseEvent){
	  astroObjectCell->clear();
	  astroObjectCell->setHidden(true);
	  toggleMoreInfo->removeStyleClass("active");
	  toggleMoreInfo->setText("&#x25bc");
	}) } ));
      };
      toggleMoreInfo->clicked().connect(std::bind(objectRow.toggleMoreInfo));
    }

    if(selection && selection.object == astroObject.object) {
      objectAddedRow = row;
      row->addStyleClass(selection.css);
      selection.onSelectionFound(objectRow);
    }
    auto addColumn = [=](Column column, function<WWidget *()> createWidget) -> WTableCell* {
      auto hasColumn = std::find(begin(d->columns), end(d->columns), column);
      if(hasColumn == end(d->columns))
        return nullptr;
      return WW<WTableCell>(row->elementAt(distance(begin(d->columns), hasColumn))).add( WW<WWidget>(createWidget()).addCss("hidden-print") ).add( WW<WWidget>(createWidget()).addCss("visible-print printable-small-text")).get();
    };
    addColumn(Names, [=] {
      auto popup = new ObjectPopupMenu{{astroObject.astroSession, astroObject.object, telescope, timezone}, d->session};
      popup->objectsListChanged().connect([=](const AstroSessionObjectPtr &o, _n5) { d->objectsListChanged.emit(o); } );
      return astroObject.names(d->session, popup, [=](WMouseEvent){
	if(d->selectedRow)
	  d->selectedRow->removeStyleClass("info");
	if(objectAddedRow)
	  objectAddedRow->removeStyleClass(selection.css);
	row->addStyleClass("info");
	d->selectedRow = row;
      });
    });
    addColumn(Type, [=] { return new WText{astroObject.typeDescription() }; });
    addColumn(AR, [=] { return new WText{ Utils::htmlEncode( astroObject.ar().printable(Angle::Hourly)) }; });
    addColumn(DEC, [=] { return new WText{ Utils::htmlEncode( WString::fromUTF8( astroObject.dec().printable() )) }; });
    addColumn(Constellation, [=] { return new WText{ WString::fromUTF8(astroObject.constellation().name) }; });
    addColumn(AngularSize, [=] { return new WText{ Utils::htmlEncode( WString::fromUTF8( astroObject.angularSize() > Angle::degrees(0) ? astroObject.angularSize().printable() : "N/A" ) )}; });
    addColumn(Magnitude, [=] { return new WText{ astroObject.magnitude() > 90. ? "N/A" : (format("%.1f") % astroObject.magnitude()).str() }; });
    addColumn(ObservationTime, [=] { return new WText{ astroObject.transitTime().str() }; });
    addColumn(RiseTime, [=] { return new WText{ astroObject.rts().rise.str() }; });
    addColumn(TransitTime, [=] { return new WText{ astroObject.rts().transit.str() }; });
    addColumn(SetsTime, [=] { return new WText{ astroObject.rts().set.str() }; });
    addColumn(MaxAltitude, [=] { return new WText{ Utils::htmlEncode(WString::fromUTF8( astroObject.maxAltitude().printable() )) }; });
    addColumn(Difficulty, [=] { return astroObject.difficultyWidget(telescope); }); 
    
    auto createButton = [=,&objectRow] (const Action &action) {
      WPushButton *button = WW<WPushButton>(WString::tr(action.name)).addCss("btn-xs").addCss(action.buttonCss);
      objectRow.actions[action.name] = button;
      button->clicked().connect([=](WMouseEvent) { action.onClick(objectRow, button); });
      action.onButtonCreated(button, objectRow);
      return button;
    };
    
    if(d->actions.size() > 0) {
      if(d->forceActionsAsToolBar || d->actions.size() == 1) {
	WToolBar *toolbar = WW<WToolBar>();
	row->elementAt(d->columns.size())->addWidget(toolbar);
	for(auto action: d->actions) {
	  toolbar->addButton(createButton(action));
	}
      } else {
	WPopupMenu *actionsMenu = new WPopupMenu;
	WPushButton *actionsButton = WW<WPushButton>(WString::tr("buttons_actions")).css("btn-xs").onClick([=](WMouseEvent e) {actionsMenu->popup(e); });
	row->elementAt(d->columns.size())->addWidget(actionsButton);
	for(auto action: d->actions) {
	  auto menuItem = actionsMenu->addItem(WString::tr(action.name));
	  objectRow.actions[action.name] = menuItem;
	  menuItem->addStyleClass(action.buttonCss);
	  menuItem->triggered().connect([=](WMenuItem*, _n5) { action.onClick(objectRow, menuItem); });
	  action.onMenuItemCreated(menuItem, objectRow);
	}
      }
      row->elementAt(d->columns.size())->addStyleClass("hidden-print");
    }
    astroObjectCell->setColumnSpan(d->columns.size() + (d->actions.size() ? 1 : 0));
    d->rows.push_back(objectRow);
  }
  if(page && page.total > 1) {
    WContainerWidget *paginationWidget = WW<WContainerWidget>().addCss("pagination pagination-sm");
    paginationWidget->setList(true);
    d->tableFooter->addWidget(paginationWidget);
    d->tableFooter->addWidget(new WBreak);
    
    auto activatePage = [=](int pageNumber) {
      clearSelection();
      if(pageNumber<0 || pageNumber>=page.total) return;
      page.change(pageNumber);
      wApp->doJavaScript(format(R"(
	$(document.body).animate({
	  'scrollTop':   $('#%s').offset().top
	  }, 500);)") % d->objectsTable->id() );
    };

    auto add = [=] (const WString &text, bool condition, int newPage) {
      WAnchor *link = WW<WAnchor>("", text);
      link->addStyleClass("link");
      if(condition)
        link->clicked().connect([=](WMouseEvent){ if(condition) activatePage(newPage); });
      paginationWidget->addWidget(WW<WContainerWidget>().addCss(condition ? "pagination-item" : "pagination-item disabled").addCss(newPage == page.current ? "active" : "").add(link));
    };
    
    long start = 0;
    long paginationMax = 15;
    if(page.current> (paginationMax/2-1) && page.total > paginationMax) {
      start = page.current - (paginationMax/2-1);
    }
    int pEnd = min(start + paginationMax, page.total);
    add("&laquo;", start>0, 0);
    add(format("-%d") % paginationMax, page.current > paginationMax, page.current - paginationMax);
    add("&lt;", page.current > 0, page.current - 1);
    for(int i=start; i <pEnd; i++) {
      add(format("%d") % (i+1), true, i);
    }
    add("&gt;", page.current < page.total-1, page.current+1);
    add(format("+%d") % paginationMax, page.current+paginationMax < page.total, page.current+paginationMax);
    add("&raquo;", pEnd < page.total, page.total-1);
  }
}

AstroObjectsTable::Page AstroObjectsTable::Page::fromCount( long pageNumber, long count, function<void(long)> onChange, size_t pageSize )
{
  Page page;
  page.current = pageNumber;
  page.total = count / pageSize + (count % pageSize ? 1 : 0);
  page.pageSize = pageSize;
  page.change = onChange;
  return page;
}
long AstroObjectsTable::Page::offset() const
{
  return current * pageSize;
}


WContainerWidget *AstroObjectsTable::tableFooter() const
{
  return d->tableFooter;
}

Signal<AstroSessionObjectPtr> &AstroObjectsTable::objectsListChanged() const
{
  return d->objectsListChanged;
}


void AstroObjectsTable::forceActionsAsToolBar(bool force)
{
  d->forceActionsAsToolBar = force;
}

vector< AstroObjectsTable::Row > AstroObjectsTable::rows() const
{
  return d->rows;
}

ostream& operator<<(ostream& o, const AstroObjectsTable::Filters& f)
{
  /*
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
  */
  o << "AstroObjectsTable::Filters{ types: [";
  string sep;
  for(auto type: f.types) {
    o << sep << NgcObject::typeDescriptionKey(type);
    sep = ", ";
  }
  o << "], ";
  o << format("minimumMagnitude: %.2f, maximumMagnitude: %.2f, ") % f.minimumMagnitude % f.maximumMagnitude;
  o << "constellation: " << f.constellation.abbrev << ", ";
  o << "catalogue: " << string{f.catalogue ? f.catalogue->name() : "<null>"} << ", ";
  o << "minimumAltitude: " << f.minimumAltitude.printable() << " (valid: " << boolalpha << f.minimumAltitude.valid() 
    << ", maximumAltitude: " << f.maximumAltitude.printable() << " (valid: " << boolalpha << f.maximumAltitude.valid() << "), ";
  o << "start_time: " << boost::posix_time::to_iso_extended_string(f.start_time) << ", end_time: " << boost::posix_time::to_iso_extended_string(f.end_time) << ", ";
  o << "observed: " << f.observed << ", ";
  o << "minimumAngularSize: " << f.minimumAngularSize.printable() << " (valid: " << f.minimumAngularSize.valid()
    << "), maximumAngularSize: " << f.maximumAngularSize.printable() << " (valid: " << f.maximumAngularSize.valid() << ") ";
  o << " }";
  return o;
}


