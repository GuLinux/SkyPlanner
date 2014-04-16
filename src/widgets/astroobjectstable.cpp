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

#include "astroobjectstable.h"
#include "private/astroobjectstable_p.h"
#include "utils/d_ptr_implementation.h"
#include <Wt/WContainerWidget>
#include <Wt/WText>
#include <Wt/WPushButton>
#include <Wt/Utils>
#include <Wt/WPopupMenu>
#include <Wt-Commons/wt_helpers.h>
#include "models/Models"
#include "widgets/astroobjectwidget.h"
#include "widgets/objectnameswidget.h"
#include "utils/format.h"
#include "widgets/objectdifficultywidget.h"

using namespace Wt;
using namespace WtCommons;
using namespace std;

AstroObjectsTable::Private::Private( Session &session, const vector< AstroObjectsTable::Action > &actions, const vector< AstroObjectsTable::Column > &columns, AstroObjectsTable *q )
 : session(session), actions(actions), columns(columns), q(q)
{
}

AstroObjectsTable::AstroObjectsTable(Session &session, const vector<Action> &actions, bool showFilters, const set<NgcObject::NebulaType> &initialTypes, const vector<Column> &columns, WContainerWidget *parent)
  : WCompositeWidget(parent), d(session, actions, columns, this)
{
  d->objectsTable = WW<WTable>().addCss("table table-hover astroobjects-table");
  d->objectsTable->setHeaderCount(1);
  WContainerWidget *container = WW<WContainerWidget>();
  if(showFilters) {
    d->filterByType = new FilterByTypeWidget(initialTypes);
    d->filterByType->changed().connect([=](_n6){ d->filtersChanged.emit(d->filters()); });
    d->filterByMinimumMagnitude = new FilterByMagnitudeWidget({WString::tr("not_set"), {}, WString::tr("minimum_magnitude_label")}, {0, 20});
    d->filterByMinimumMagnitude->changed().connect([=](double, _n5){ d->filtersChanged.emit(d->filters()); });

    d->filterByCatalogue = new FilterByCatalogue(session);
    d->filterByCatalogue->changed().connect([=](_n6){ d->filtersChanged.emit(d->filters()); });
   
    d->filterByConstellation = new FilterByConstellation;
    d->filterByConstellation->changed().connect([=](_n6){ d->filtersChanged.emit(d->filters()); });
    container->addWidget(WW<WContainerWidget>().addCss("form-inline").add(d->filterByType).add(d->filterByMinimumMagnitude).add(d->filterByConstellation).add(d->filterByCatalogue));
  }
  container->addWidget(WW<WContainerWidget>().addCss("table-responsive").add(d->objectsTable).add(d->tableFooter = WW<WContainerWidget>() ));
  setImplementation(container);
}

AstroObjectsTable::Filters AstroObjectsTable::Private::filters() const
{
  Filters _filters;
  if(!filterByMinimumMagnitude->isMinimum())
    _filters.minimumMagnitude = filterByMinimumMagnitude->magnitude();
  _filters.catalogue = filterByCatalogue->selectedCatalogue();
  _filters.constellation = filterByConstellation->selectedConstellation();
  _filters.types = filterByType->selected();
  return _filters;
}

void AstroObjectsTable::setMaximumMagnitude( double magnitudeLimit )
{
  d->filterByMinimumMagnitude->setMaximum(magnitudeLimit);
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

const vector<AstroObjectsTable::Column> AstroObjectsTable::allColumns = {
  Names, Type, AR, DEC, Constellation, AngularSize, Magnitude, TransitTime, MaxAltitude, Difficulty
};

void AstroObjectsTable::Private::header()
{
  static map<Column, string> columnKey {
    {Names, "object_column_names"},
    {Type, "object_column_type"},
    {AR, "object_column_ar"},
    {DEC, "object_column_dec"},
    {Constellation, "object_column_constellation"},
    {AngularSize, "object_column_angular_size"},
    {Magnitude, "object_column_magnitude"},
    {TransitTime, "object_column_highest_time"},
    {MaxAltitude, "object_column_max_altitude"},
    {Difficulty, "object_column_difficulty"},
  };
  q->clear();
  int index = 0;
  for(auto column: columns) {
    objectsTable->elementAt(0,index++)->addWidget(new WText{WString::tr(columnKey[column])});
  }
}

void AstroObjectsTable::populate(const vector<AstroObject> &objects, const TelescopePtr &telescope, const Timezone &timezone, const Page &page, const Selection &selection)
{
  auto clearSelection = [=] {
    d->selectedRow = nullptr;
  };
  clearSelection();
  d->header();
  d->tableFooter->clear();
  WTableRow *objectAddedRow = nullptr;
  for(auto astroObject: objects) {
    WTableRow *row = d->objectsTable->insertRow(d->objectsTable->rowCount());
    row->addStyleClass(astroObject.rowStyle);
    WTableRow *astroObjectRow = d->objectsTable->insertRow(d->objectsTable->rowCount());
    WTableCell *astroObjectCell = astroObjectRow->elementAt(0);
    astroObjectCell->setHidden(true);
    WPushButton *toggleMoreInfo = WW<WPushButton>(row->elementAt(0)).css("btn btn-xs pull-right hidden-print").setTextFormat(XHTMLUnsafeText).setText("&#x25bc;").setAttribute("title", WString::tr("astroobject_extended_info_title").toUTF8() );

    Row objectRow{astroObject, row};
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
      astroObjectCell->addWidget(new AstroObjectWidget(astroObject.object, astroObject.astroSession, d->session, timezone, telescope, {}, {WW<WPushButton>(WString::tr("buttons_close")).css("btn-xs").onClick([=](WMouseEvent){
        astroObjectCell->clear();
        astroObjectCell->setHidden(true);
        toggleMoreInfo->removeStyleClass("active");
        toggleMoreInfo->setText("&#x25bc");
      }) } ));
    };
    toggleMoreInfo->clicked().connect(std::bind(objectRow.toggleMoreInfo));

    if(selection && selection.object == astroObject.object) {
      objectAddedRow = row;
      row->addStyleClass(selection.css);
      selection.onSelectionFound(objectRow);
    }
    auto addColumn = [=](Column column, function<WWidget *()> createWidget) -> WTableCell* {
      auto hasColumn = std::find(begin(d->columns), end(d->columns), column);
      if(hasColumn == end(d->columns))
        return nullptr;
      return WW<WTableCell>(row->elementAt(hasColumn - begin(d->columns))).add(createWidget() ).get();
    };
    addColumn(Names, [=] { return WW<ObjectNamesWidget>(new ObjectNamesWidget{astroObject.object, d->session, astroObject.astroSession}).setInline(true).onClick([=](WMouseEvent){
      if(d->selectedRow)
        d->selectedRow->removeStyleClass("info");
      if(objectAddedRow)
        objectAddedRow->removeStyleClass(selection.css);
      row->addStyleClass("info");
      d->selectedRow = row;
    }).get(); });
    addColumn(Type, [=] { return new WText{astroObject.object->typeDescription() }; });
    addColumn(AR, [=] { return new WText{ Utils::htmlEncode( astroObject.object->coordinates().rightAscension.printable(Angle::Hourly) ) }; });
    addColumn(DEC, [=] { return new WText{ Utils::htmlEncode( WString::fromUTF8( astroObject.object->coordinates().declination.printable() )) }; });
    addColumn(Constellation, [=] { return new WText{ WString::fromUTF8(astroObject.object->constellation().name) }; });
    addColumn(AngularSize, [=] { return new WText{ Utils::htmlEncode( WString::fromUTF8( Angle::degrees(astroObject.object->angularSize()).printable() )) }; });
    addColumn(Magnitude, [=] { return new WText{ astroObject.object->magnitude() > 90. ? "N/A" : (format("%.1f") % astroObject.object->magnitude()).str() }; });
    auto bestAltitude = astroObject.bestAltitude;
    addColumn(TransitTime, [=] { return new WText{ bestAltitude.when.str() }; });
    addColumn(MaxAltitude, [=] { return new WText{ Utils::htmlEncode(WString::fromUTF8(bestAltitude.coordinates.altitude.printable() )) }; });
    addColumn(Difficulty, [=] { return new ObjectDifficultyWidget{astroObject.object, telescope, bestAltitude.coordinates.altitude.degrees() }; }); 
    
    if(d->actions.size() > 0) {
      if(d->actions.size() == 1) {
        row->elementAt(d->columns.size())->addWidget(WW<WPushButton>(WString::tr(d->actions[0].name)).addCss("btn-xs").addCss(d->actions[0].buttonCss).onClick([=](WMouseEvent) { d->actions[0].onClick(objectRow); }));
      } else {
        WPopupMenu *actionsMenu = new WPopupMenu;
        WPushButton *actionsButton = WW<WPushButton>(WString::tr("buttons_actions")).css("btn-xs");
        actionsButton->setMenu(actionsMenu);
        row->elementAt(d->columns.size())->addWidget(actionsButton);
        for(auto action: d->actions) {
          auto menuItem = actionsMenu->addItem(WString::tr(action.name));
          menuItem->addStyleClass(action.buttonCss);
          menuItem->triggered().connect([=](WMenuItem*, _n5) { action.onClick(objectRow); });
          action.onMenuItemCreated(menuItem, objectRow);
        }
      }
    }
    astroObjectCell->setColumnSpan(d->columns.size() + (d->actions.size() ? 1 : 0));
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
    };

    auto add = [=] (const WString &text, bool condition, int newPage) {
      WAnchor *link = WW<WAnchor>("", text);
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
