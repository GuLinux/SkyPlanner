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

#ifndef PLACEWIDGET_H
#define PLACEWIDGET_H

#include <Wt/WContainerWidget>
#include "utils/d_ptr.h"

class AstroSession;
class Session;
class PlaceWidget : public Wt::WContainerWidget
{
public:
    ~PlaceWidget();
    PlaceWidget(const Wt::Dbo::ptr<AstroSession> &astroSession, Session &session, Wt::WContainerWidget* parent = 0);
    Wt::Signal<double,double> &placeChanged() const;
private:
    D_PTR;
};

#endif // PLACEWIDGET_H