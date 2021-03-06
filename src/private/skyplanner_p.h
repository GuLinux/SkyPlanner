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

#ifndef ASTROPLANNER_P_H
#define ASTROPLANNER_P_H
#include "skyplanner.h"
#include "session.h"

class AstroSessionsPage;
class AstroObjectsTable;
namespace Wt
{
  class WMenuItem;
}

class SkyPlanner::Private
{
public:
    Private(SkyPlanner* q, OnQuit onQuit);
    OnQuit onQuit;
    Session session;
    
    Notifications::ptr notifications;
    Wt::WStackedWidget *widgets;
    void loadDSSPage(const std::string &hexId);
    void loadReport(const std::string &hexId);
    void loadPreview(const std::string &hexId);
    bool searchByName(const std::string &name, AstroObjectsTable *table, int page = 0);
    std::string previousInternalPath = "/";
    Wt::WContainerWidget *dssContainer;
    Wt::WContainerWidget *reportsContainer;
    Wt::WString loginname;
    Wt::Signal<> telescopesListChanged;
    std::string lastNameSearch;
    bool agentIsBot;
    std::string initialInternalPath;
    SessionInfo sessionInfo;
    AstroSessionsPage* astrosessionspage;
private:
    class SkyPlanner* const q;
};

#endif // ASTROPLANNER_P_H
