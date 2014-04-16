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

#ifndef ASTROPLANNER_H
#define ASTROPLANNER_H

#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include "utils/d_ptr.h"

class SkyPlanner : public Wt::WApplication
{
public:
  SkyPlanner(const Wt::WEnvironment& environment);
    ~SkyPlanner();
    class Notification {
    public:
      enum Type { Alert, Error, Success, Information };
      Notification(const Wt::WString &title, const Wt::WString &content, Type type, bool addCloseButton, Wt::WContainerWidget *parent = 0);
      ~Notification();
      void close();
      bool valid() const;
      Wt::Signal<> &closed() const;
      Wt::WWidget *widget() const;
    private:
      D_PTR;
    };
    friend class Notification;
    static SkyPlanner *instance();
    std::shared_ptr<Notification>notification( const Wt::WString &title, const Wt::WString &content, Notification::Type type, int autoHideSeconds = 0 , Wt::WContainerWidget *addTo = nullptr);
    static const std::string HOME_PATH;
    Wt::WLogEntry uLog (const std::string &type) const;

    Wt::Signal<> &telescopesListChanged() const;
private:
    D_PTR;
};

#define spLog(type) SkyPlanner::instance()->uLog(type) << __PRETTY_FUNCTION__ << "-"

#endif // ASTROPLANNER_H
