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

#ifndef ASTROSESSIONTAB_P_H
#define ASTROSESSIONTAB_P_H
#include "astrosessiontab.h"
#include <forecast.h>
#include "types.h"
#include "models/Models"
#include "geocoder.h"

#include "exportastrosessionresource.h"
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
class FilterByMagnitudeWidget;
class FilterByCatalogue;
class FilterByTypeWidget;
class AstroObjectsTable;
class AstroSessionTab::Private
{
public:

    Private(const Wt::Dbo::ptr<AstroSession>& astroSession, Session& session, AstroSessionTab* q);
    void reload();
    Wt::Dbo::ptr<AstroSession> astroSession;
    Session &session;
    Wt::WContainerWidget *positionDetails;
    void populate(const Wt::Dbo::ptr<AstroSessionObject> &astroSessionObject = {}, int pageNumber = 0);
    Wt::WPanel *addPanel(const Wt::WString &title, Wt::WWidget *widget, bool collapsed = false, bool collapsible = true, Wt::WContainerWidget *container = 0);
    void updatePositionDetails(Wt::WContainerWidget *positionDetails, bool showMeteo = true);
    Forecast forecast;
    Wt::Dbo::ptr<Telescope> selectedTelescope;
    Wt::Signal<std::string> nameChanged;
    void printableVersion();
    bool pastObservation;
    std::shared_ptr<Wt::Http::Client> client;
    Timezone timezone;
    void updateTimezone();
    Wt::Signal<> close;
    std::map<ExportAstroSessionResource::ReportType, ExportAstroSessionResource*> exportResources;
    void remove(const Wt::Dbo::ptr<AstroSessionObject> &sessionObject, std::function<void()> runAfterRemove);
    Wt::WText *objectsCounter;
    AstroObjectsTable *astroObjectsTable;
    struct SetDescription {
      typedef std::function<void(Wt::Dbo::Transaction &t, const Wt::WString &txt)> EditTextField;
      typedef std::function<std::string(Wt::Dbo::Transaction &t)> GetTextField;
      SetDescription(const std::string &title, const std::string &notification, GetTextField getDescription, EditTextField editTextField, std::function<void()> onUpdate)
      : title(title), notification(notification), getDescription(getDescription), editTextField(editTextField), onUpdate(onUpdate) {}
      static SetDescription description(const AstroSessionObjectPtr &o, std::function< void() > onUpdate = []{}) {
        return {"object_notes", "notification_description_saved", [=](Wt::Dbo::Transaction &t){ return o->description(); },
        [=](Wt::Dbo::Transaction &t, const Wt::WString &txt){ o.modify()->setDescription(txt.toUTF8());}, onUpdate};
      }
      template<typename T>
      static SetDescription report(const T &o, std::function< void() > onUpdate = []{}, std::string title = "report") {
        return {title, "notification_report_saved", [=](Wt::Dbo::Transaction &t){ return o->report() ? *o->report() : std::string{}; },
        [=](Wt::Dbo::Transaction &t, const Wt::WString &txt){ o.modify()->setReport(txt.toUTF8());}, onUpdate};
      }
      std::string title;
      std::string notification;
      EditTextField editTextField;
      GetTextField getDescription;
      std::function< void() > onUpdate = [] {};
    };
    void setDescriptionDialog(const SetDescription &setDescription);
    GeoCoder::Place geoCoderPlace;
    void previewVersion(bool isReport = false);
    Wt::WStackedWidget *sessionStacked ;
    Wt::WContainerWidget *sessionContainer ;
    Wt::WContainerWidget *sessionPreviewContainer;
    void populatePlanets(AstroObjectsTable *planetsTable);
private:
    class AstroSessionTab* const q;
};


#endif // ASTROSESSIONTAB_P_H
