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
#ifndef ASTROPLANNER_H
#define ASTROPLANNER_H

#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include "c++/dptr.h"
#include "json_object.h"
#include "widgets/notification.h"
#include "widgets/notifications.h"

class SkyPlanner : public Wt::WApplication
{
public:
    struct SessionInfo  : public WtCommons::Json::Object {
      SessionInfo();
      std::string sessionId;
      std::string ipAddress;
      std::string userAgent;
      std::string username;
      std::string referrer;
      boost::posix_time::ptime started;
      boost::posix_time::ptime lastEvent;
    };

    typedef std::function<void(SkyPlanner*)> OnQuit;
  SkyPlanner(const Wt::WEnvironment& environment, OnQuit onQuit);
    ~SkyPlanner();
    static SkyPlanner *instance();
    Notifications::ptr notifications() const;
    Wt::WLogEntry uLog (const std::string &type) const;

    Wt::Signal<> &telescopesListChanged() const;
    SessionInfo sessionInfo() const;
    protected:
    virtual void notify(const Wt::WEvent &e);
private:
    D_PTR;
};

#define spLog(type) SkyPlanner::instance()->uLog(type) << __PRETTY_FUNCTION__ << "-"

#endif // ASTROPLANNER_H
