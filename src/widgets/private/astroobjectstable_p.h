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

#ifndef ASTROOBJECTSTABLE_P_H
#define ASTROOBJECTSTABLE_P_H
#include "widgets/astroobjectstable.h"
#include <widgets/filterbytypewidget.h>
#include <widgets/filterbymagnitudewidget.h>
#include <widgets/filterbycatalogue.h>
#include <widgets/filterbyaltitudewidget.h>
#include <widgets/filterbyconstellation.h>
#include <widgets/filterbyobservedwidget.h>
#include "session.h"
#include <Wt/WTable>
#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>
#include <Wt/WComboBox>

class AstroObjectsTable::Private
{
  public:
    Private(Session &session, const std::vector<AstroObjectsTable::Action> &actions, const std::list<Column> &columns, AstroObjectsTable *q );
    Session &session;
    std::vector<AstroObjectsTable::Action> actions; 
    std::list<AstroObjectsTable::Column> columns;
    Wt::WTable *objectsTable;
    Wt::WTableRow *selectedRow = nullptr;

    Wt::Signal<Filters> filtersChanged;
    Filters filters() const;
    void header();
    Wt::WContainerWidget *tableFooter;
    Wt::WContainerWidget *tableContainer;
    Wt::Signal<AstroSessionObjectPtr> objectsListChanged;
    Wt::WPopupMenu *availableFilters;
    Wt::WContainerWidget *filtersBar;
    Wt::WPushButton *filtersButton = nullptr;
    template<typename T> void addFilterItem(const std::string &text, T *filterWidget);

    bool forceActionsAsToolBar = false;
    
    struct FilterView {
      std::string menuItemName;
      Wt::WMenuItem *menuItem;
      Wt::WWidget* filter;
      Wt::WContainerWidget *container = nullptr;
      std::function<void()> remove;
      FilterView(const std::string &itemName, Wt::WMenuItem *menuItem, Wt::WWidget *filter, std::function<void()> remove)
	: menuItemName(itemName), menuItem(menuItem), filter(filter), remove(remove) {}
      FilterView() = default;
    };
    std::map<std::string,FilterView> filterViews;
    template<typename T> T* filter(const std::string &name) const;
    std::vector<Row> rows;
  private:
    class AstroObjectsTable *const q; 
};

#endif
