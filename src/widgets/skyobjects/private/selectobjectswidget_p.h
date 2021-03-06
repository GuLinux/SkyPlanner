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

#ifndef SELECTOBJECTSWIDGET_P_H
#define SELECTOBJECTSWIDGET_P_H
#include "widgets/skyobjects/selectobjectswidget.h"
#include <boost/thread.hpp>
#include "utils/format.h"
#include "widgets/skyobjects/astroobjectstable.h"

class FilterByConstellation;
namespace Wt
{
  class WTableRow;
}

class NgcObject;
class SelectObjectsWidget::Private
{
public:
    Private(const Wt::Dbo::ptr<AstroSession>& astroSession, Session& session, SelectObjectsWidget* q);
    Wt::Dbo::ptr<AstroSession> astroSession;
    Session &session;
    Wt::Signal<Wt::Dbo::ptr<AstroSessionObject>> objectsListChanged;
    AstroObjectsTable *suggestedObjectsTable;
    Wt::WToolBar *suggestedObjectsToolbar;
    Wt::WComboBox *suggestedSortByCombo;
    Wt::WStandardItemModel *suggestedObjectsSortModel;
    Wt::WPushButton *suggestedObjectsAddAllButton = nullptr;
    void searchByCatalogueTab(Wt::Dbo::Transaction &transaction);
    void searchByNameTab(Wt::Dbo::Transaction &transaction);
    void suggestedObjects(Wt::Dbo::Transaction &transaction);
    void populateSuggestedObjectsTable(int pageNumber = 0);
    boost::mutex suggestedObjectsListMutex;
    int pagesCurrentIndex = 0;
    Wt::Dbo::ptr< Telescope > selectedTelescope;
    Timezone timezone;
    std::string lastSearch;
    Wt::WTableRow *selectedRow = 0;
    void addToSession(const Wt::Dbo::ptr<NgcObject> &object, Wt::WTableRow *row);
    AstroObjectsTable::Action addToSessionAction;
    void searchByName(const std::string &name, AstroObjectsTable *table, int page = 0);
    std::string styleFor(const NgcObjectPtr &object, Wt::Dbo::Transaction &t) const;
    std::list<AstroObjectsTable::Column> columns;
private:
    class SelectObjectsWidget* const q;
};


#endif // SELECTOBJECTSWIDGET_P_H
