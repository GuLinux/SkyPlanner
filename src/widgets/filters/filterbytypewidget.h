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
#ifndef FILTERBYTYPEWIDGET_H
#define FILTERBYTYPEWIDGET_H

#include <c++/dptr.h>
#include <Wt/WCompositeWidget>
#include <set>
#include "models/ngcobject.h"

class FilterByTypeWidget : public Wt::WCompositeWidget {
public:
  explicit FilterByTypeWidget(const std::set<NgcObject::NebulaType> &initialSelection, Wt::WContainerWidget *parent = 0);
  ~FilterByTypeWidget();
  Wt::Signal<> &changed() const;
  std::set<NgcObject::NebulaType> selected() const;
  void resetDefaultValue();
private:
  D_PTR;
};

#endif // FILTERBYTYPEWIDGET_H


