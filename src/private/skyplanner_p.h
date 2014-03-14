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

#ifndef ASTROPLANNER_P_H
#define ASTROPLANNER_P_H
#include "skyplanner.h"
#include "session.h"

namespace Wt
{
  class WMenuItem;
}

class SkyPlanner::Private
{
public:
    Private(SkyPlanner* q);
    Session session;
    
    std::vector<Wt::WMenuItem*> loggedInItems;
    std::vector<Wt::WMenuItem*> loggedOutItems;
    Wt::WContainerWidget *notifications;
    Wt::WStackedWidget *widgets;
    void loadDSSPage(const std::string &hexId);
    std::string previousInternalPath = "/";
    Wt::WContainerWidget *dssContainer;
    Wt::WString loginname;
private:
    class SkyPlanner* const q;
};

#endif // ASTROPLANNER_P_H