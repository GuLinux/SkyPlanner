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
    struct ObjectSessionData {
      struct Key {
        Coordinates::LatLng position;
        boost::posix_time::ptime when;
        long objectId;
        std::string str() const { return format("%.3f-%.3f-%s-%d") % position.latitude.degrees() % position.longitude.degrees() % boost::posix_time::to_simple_string(when) % objectId; }
        bool operator<(const Key &other) const { return str() < other.str(); }
      };
      NgcObjectPtr object;
      Ephemeris::BestAltitude bestAltitude;
      double observabilityIndex;
      operator bool() { return object && bestAltitude.coordinates && bestAltitude.when != boost::posix_time::ptime(); }

    };

    Private(const Wt::Dbo::ptr<AstroSession>& astroSession, Session& session, SelectObjectsWidget* q);
    Wt::Dbo::ptr<AstroSession> astroSession;
    Session &session;
    Wt::Signal<> objectsListChanged;
    Wt::WTable *suggestedObjectsTable;
    Wt::WContainerWidget *suggestedObjectsTablePagination;
    void searchByCatalogueTab(Wt::Dbo::Transaction &transaction);
    void searchByNameTab(Wt::Dbo::Transaction &transaction);
    void suggestedObjects(Wt::Dbo::Transaction &transaction);
    void populateSuggestedObjectsTable();
    void populateSuggestedObjectsList( double magnitudeLimit );
    Wt::Signal<> suggestedObjectsLoaded;
    typedef std::vector<ObjectSessionData> NgcObjectsList; 
    NgcObjectsList suggestedObjectsList; 
    boost::mutex sessionLockMutex;
    boost::mutex suggestedObjectsListMutex;
    int pagesCurrentIndex = 0;
    Wt::Dbo::ptr< Telescope > selectedTelescope;
    Timezone timezone;
    void populateHeaders(Wt::WTable *table);
    void append(Wt::WTable *table, const Wt::Dbo::ptr<NgcObject> &ngcObject, const Ephemeris::BestAltitude &bestAltitude);
    boost::thread bgThread;
    bool aborted = false;
    FilterByTypeWidget *filterByTypeWidget;
    FilterByMagnitudeWidget *filterByMinimumMagnitude;
    std::string lastSearch;
    Wt::WTableRow *selectedRow = 0;
    std::map<ObjectSessionData::Key,ObjectSessionData> objectsSessionDataCache;
    std::map<int64_t, NgcObjectPtr> objectsCache;
private:
    class SelectObjectsWidget* const q;
};

#endif // SELECTOBJECTSWIDGET_P_H
