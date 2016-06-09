/*
 * Copyright (C) 2016  Marco Gulino <marco.gulino@gmail.com>
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

#ifndef NOTIFICATIONS_H
#define NOTIFICATIONS_H

#include <Wt/WContainerWidget>
#include "c++/dptr.h"
#include "notification.h"
class Notifications : public Wt::WContainerWidget
{
public:
    typedef std::shared_ptr<Notifications> ptr;
    static ptr instance();
    
    ~Notifications();
    Notifications(Wt::WContainerWidget* parent);
    Notification::ptr show( const Wt::WString &title, const Wt::WString &content, Notification::Type type, int autoHideSeconds = 0, const std::string &categoryTag = {});
    Notification::ptr show( const Wt::WString &title, Wt::WWidget *content, Notification::Type type, int autoHideSeconds = 0, const std::string &categoryTag = {});
    void clearNotifications();

private:
    D_PTR
};

#endif // NOTIFICATIONS_H
