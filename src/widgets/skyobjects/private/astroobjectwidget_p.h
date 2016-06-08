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
#ifndef ASTROOBJECTWIDGET_P_H
#define ASTROOBJECTWIDGET_P_H

#include "widgets/skyobjects/astroobjectwidget.h"
#include "models/Models"
#include "session.h"
class DSSPage;
class AstroObjectWidget::Private
{
public:
  Private(const AstroGroup &astroGroup, Session &session, const std::shared_ptr<std::mutex> &downloadMutex, const std::vector<Wt::WPushButton*> &actionButtons, AstroObjectWidget *q);

  void init();

  AstroGroup astroGroup;
  Session &session;
  std::shared_ptr<std::mutex> downloadMutex;
  std::vector<Wt::WPushButton*> actionButtons;
  

  DSSPage *dssPage;
  Wt::WContainerWidget *content;
  Wt::WContainerWidget *expanded;
  Wt::WWidget *collapsed;
  Wt::WTemplate *info;
  Wt::WContainerWidget *actionsToolbar;
private:
  AstroObjectWidget *q;
};

#endif // ASTROOBJECTWIDGET_H


