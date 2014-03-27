/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Marco Gulino <email>
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

#ifndef OBJECTNAMESWIDGET_H
#define OBJECTNAMESWIDGET_H

#include <Wt/WContainerWidget>
#include "utils/d_ptr.h"
class NgcObject;
class AstroSessionObject;
class AstroSession;
class Timezone;
class Telescope;
class Session;
class ObjectNamesWidget : public Wt::WContainerWidget {
public:
  enum RenderType {Normal, Printable};
  ObjectNamesWidget(const Wt::Dbo::ptr<NgcObject> &object, Session &session, const Wt::Dbo::ptr<AstroSession> &astroSession, RenderType renderType = Normal, int limitNames = 0, Wt::WContainerWidget *parent = 0);
  ObjectNamesWidget(const Wt::Dbo::ptr<AstroSessionObject> &object, const Timezone &timezone, const Wt::Dbo::ptr<Telescope> &telescope, Session &session, RenderType renderType = Normal, int limitNames = 0, Wt::WContainerWidget *parent = 0);
  ~ObjectNamesWidget();
private:
  D_PTR;
};
#endif
