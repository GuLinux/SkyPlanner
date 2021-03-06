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

#ifndef ASTROSESSIONTAB_P_H
#define ASTROSESSIONTAB_P_H
#include "widgets/pages/astrosessiontab.h"
#include <forecast.h>
#include "types.h"
#include "models/Models"
#include "geocoder.h"

#include "exportastrosessionresource.h"

class SelectObjectsWidget;
class FilterByConstellation;
namespace Wt
{
  namespace Http
  {
    class Client;
  }
  class WTableRow;
}

class Session;
class ExportAstroSessionResource;
class FilterByCatalogue;
class FilterByTypeWidget;
class AstroObjectsTable;
class WeatherWidget;
class AstroSessionTab::Private
{
public:

    Private(const Wt::Dbo::ptr<AstroSession>& astroSession, Session& session, AstroSessionTab* q);
    void load();
    void updatePosition();
    Wt::WToolBar *actionsToolbar();
    Wt::Dbo::ptr<AstroSession> astroSession;
    Session &session;
    Wt::WContainerWidget *positionDetails;
    void populate(const Wt::Dbo::ptr<AstroSessionObject> &astroSessionObject = {}, int pageNumber = 0);
    Wt::WPanel *addPanel(const Wt::WString &title, Wt::WWidget *widget, bool collapsed = false, bool collapsible = true, Wt::WContainerWidget *container = 0);
    Forecast forecast;
    Wt::Dbo::ptr<Telescope> selectedTelescope;
    Wt::Signal<std::string> nameChanged;
    Wt::Signal<> sessionsChanged;
    void printableVersion();
    bool pastObservation;
    Timezone timezone;
    void updateTimezone();
    Wt::Signal<> close;
    std::map<ExportAstroSessionResource::ReportType, ExportAstroSessionResource*> exportResources;
    void remove(const Wt::Dbo::ptr<AstroSessionObject> &sessionObject, std::function<void()> runAfterRemove);
    Wt::WText *objectsCounter;
    AstroObjectsTable *astroObjectsTable;
    GeoCoder::Place geoCoderPlace;
    Wt::WStackedWidget *sessionStacked ;
    Wt::WContainerWidget *sessionContainer ;
    Wt::WContainerWidget *sessionPreviewContainer;
    WeatherWidget *weatherWidget;
    Wt::WContainerWidget *sessionInfoWidget;
    SelectObjectsWidget *addObjectsTabWidget;
private:
    class AstroSessionTab* const q;
};



#endif // ASTROSESSIONTAB_P_H
