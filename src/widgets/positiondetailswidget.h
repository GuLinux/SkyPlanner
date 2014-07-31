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

#ifndef POSITIONDETAILSWIDGET_H
#define POSITIONDETAILSWIDGET_H

#include <Wt/WCompositeWidget>
#include "models/ptr_defs.h"
#include "geocoder.h"

class Session;

class PositionDetailsWidget : public Wt::WCompositeWidget {
public:
  ~PositionDetailsWidget();
  PositionDetailsWidget(const AstroGroup& astroGroup, const GeoCoder::Place &geoCoderPlace, Session& session, bool showMeteo = true, Wt::WContainerWidget* parent = 0);
private:
};
#endif
 
