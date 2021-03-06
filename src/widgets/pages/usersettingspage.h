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
#ifndef USERSETTINGSPAGE_H
#define USERSETTINGSPAGE_H

#include <c++/dptr.h>
#include <Wt/WCompositeWidget>
class Session;

class UserSettingsPage : public Wt::WCompositeWidget {
public:
  explicit UserSettingsPage(Session &session, Wt::WContainerWidget *parent = 0);
  ~UserSettingsPage();
private:
  D_PTR;
};

#endif // USERSETTINGSPAGE_H


