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
#include <mutex>

class SelectObjectsWidget::Private
{
public:
    Private(const Wt::Dbo::ptr<AstroSession>& astroSession, Session& session, SelectObjectsWidget* q);
    Wt::Dbo::ptr<AstroSession> astroSession;
    Session &session;
    Wt::Signal<> objectsListChanged;
    Wt::WTable *suggestedObjectsTable;
    Wt::WContainerWidget *suggestedObjectsTablePagination;
    void searchByCatalogueTab(const std::shared_ptr<Wt::Dbo::Transaction> &transaction);
    void suggestedObjects(const std::shared_ptr<Wt::Dbo::Transaction> &transaction);
    void populateSuggestedObjectsList();
    Wt::Signal<> suggestedObjectsLoaded;
    typedef std::vector<std::pair<NgcObjectPtr,Ephemeris::BestAltitude>> NgcObjectsList; 
    std::shared_ptr<NgcObjectsList> suggestedObjectsList; 
    std::mutex suggestedObjectsListMutex;
    int pagesCurrentIndex = 0;
private:
    class SelectObjectsWidget* const q;
};
#endif // SELECTOBJECTSWIDGET_P_H
