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

#ifndef FILTERBYCONSTELLATION_H
#define FILTERBYCONSTELLATION_H

#include <Wt/WCompositeWidget>
#include "utils/d_ptr.h"
#include <constellationfinder.h>
#include <functional>

class FilterByConstellation : public Wt::WCompositeWidget
{
public:
  FilterByConstellation( Wt::WContainerWidget *parent = 0);
  virtual ~FilterByConstellation();
  ConstellationFinder::Constellation selectedConstellation() const;
  typedef std::function<bool(const ConstellationFinder::Constellation &constellation)> Filter;
  void setFilter(const Filter &filter);
  Wt::Signal<> &changed() const;
  void reload();
  void resetDefaultValue();
private:
    D_PTR;
};

#endif // FILTERBYCONSTELLATION_H
