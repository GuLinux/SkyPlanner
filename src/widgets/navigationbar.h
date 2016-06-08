/*
 * Copyright (C) 2016  <copyright holder> <email>
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

#ifndef NAVIGATIONBAR_H
#define NAVIGATIONBAR_H

#include <Wt/WNavigationBar>
#include "c++/dptr.h"

class Session;
class NavigationBar : public Wt::WNavigationBar
{
public:
    struct MenuItem {
      typedef std::list<MenuItem> list;
      std::string key, path;
      enum {LoggedIn, LoggedOut, Both} visibility;
      Wt::WWidget *widget;
      MenuItem::list children;
      std::list<std::string> css;
      Wt::WMenuItem **ptr;
      void addTo(Wt::WMenu* menu, Session& session) const;
    };
    ~NavigationBar();
    NavigationBar(const MenuItem::list &menu_items, Wt::WStackedWidget *stack, Session &session, Wt::WContainerWidget* parent = 0);

private:
    D_PTR
};

#endif // NAVIGATIONBAR_H
