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

#ifndef SELECTOBJECTSWIDGET_P_H
#define SELECTOBJECTSWIDGET_P_H
#include "selectobjectswidget.h"
#include <boost/thread.hpp>
#include "utils/format.h"

class FilterByConstellation;
namespace Wt
{
  class WTableRow;
}

class NgcObject;
class FilterByTypeWidget;
class FilterByMagnitudeWidget;
class SelectObjectsWidget::Private
{
public:
    Private(const Wt::Dbo::ptr<AstroSession>& astroSession, Session& session, SelectObjectsWidget* q);
    Wt::Dbo::ptr<AstroSession> astroSession;
    Session &session;
    Wt::Signal<Wt::Dbo::ptr<AstroSessionObject>> objectsListChanged;
    Wt::WTable *suggestedObjectsTable;
    Wt::WContainerWidget *suggestedObjectsFooter;
    void searchByCatalogueTab(Wt::Dbo::Transaction &transaction);
    void searchByNameTab(Wt::Dbo::Transaction &transaction);
    void suggestedObjects(Wt::Dbo::Transaction &transaction);
    void populateSuggestedObjectsList();
    boost::mutex suggestedObjectsListMutex;
    int pagesCurrentIndex = 0;
    Wt::Dbo::ptr< Telescope > selectedTelescope;
    Timezone timezone;
    void populateHeaders(Wt::WTable *table);
    void append(Wt::WTable *table, const Wt::Dbo::ptr<NgcObject> &ngcObject, const Ephemeris::BestAltitude &bestAltitude);
    FilterByTypeWidget *filterByTypeWidget;
    FilterByMagnitudeWidget *filterByMinimumMagnitude;
    FilterByConstellation *filterByConstellation;
    std::string lastSearch;
    Wt::WTableRow *selectedRow = 0;
    template<typename T> Wt::Dbo::Query<T> filterQuery(const std::string &query);
    void clearSelection();
private:
    class SelectObjectsWidget* const q;
};

#endif // SELECTOBJECTSWIDGET_P_H
