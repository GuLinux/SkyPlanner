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

namespace Wt
{
  namespace Http
  {
    class Client;
  }
  class WTableRow;
}

class Telescope;
class Session;
class PrintableAstroSessionResource;
class AstroSessionTab::Private
{
public:

    Private(const Wt::Dbo::ptr<AstroSession>& astroSession, Session& session, AstroSessionTab* q);
    void reload();
    Wt::Dbo::ptr<AstroSession> astroSession;
    Session &session;
    Wt::WTable *objectsTable;
    Wt::WContainerWidget *positionDetails;
    void populate();
    Wt::WPanel *addPanel(const Wt::WString &title, Wt::WWidget *widget, bool collapsed = false, bool collapsible = true, Wt::WContainerWidget *container = 0);
    void updatePositionDetails();
    Forecast forecast;
    Wt::Dbo::ptr<Telescope> selectedTelescope;
    Wt::Signal<std::string> nameChanged;
    void printableVersion();
    bool pastObservation;
    std::shared_ptr<Wt::Http::Client> client;
    Timezone timezone;
    void updateTimezone();
    Wt::WTableRow *selectedRow = 0;
    Wt::Signal<> close;
    PrintableAstroSessionResource *exportToCsvResource;
private:
    class AstroSessionTab* const q;
};


#endif // ASTROSESSIONTAB_P_H
