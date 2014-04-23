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
#include "filterbyconstellation_p.h"
#include <widgets/filterbycatalogue.h>
#include "session.h"
#include <Wt/WTable>

class AstroObjectsTable::Private
{
  public:
    Private(Session &session, const std::vector<AstroObjectsTable::Action> &actions, const std::vector<AstroObjectsTable::Column> &columns, AstroObjectsTable *q );
    Session &session;
    std::vector<AstroObjectsTable::Action> actions; 
    std::vector<AstroObjectsTable::Column> columns;
    Wt::WTable *objectsTable;
    Wt::WTableRow *selectedRow = nullptr;
    FilterByTypeWidget *filterByType;
    FilterByMagnitudeWidget *filterByMinimumMagnitude;
    FilterByConstellation *filterByConstellation;
    FilterByCatalogue *filterByCatalogue;
    Wt::Signal<Filters> filtersChanged;
    Filters filters() const;
    void header();
    Wt::WContainerWidget *tableFooter;
    Wt::WContainerWidget *tableContainer;
  private:
    class AstroObjectsTable *const q; 
};

#endif

