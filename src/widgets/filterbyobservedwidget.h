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
#ifndef FILTERBYOBSERVEDWIDGET_H
#define FILTERBYOBSERVEDWIDGET_H

#include <utils/d_ptr.h>
#include <Wt/WCompositeWidget>
#include <boost/logic/tribool.hpp>
class FilterByObservedWidget : public Wt::WCompositeWidget {
public:
  explicit FilterByObservedWidget(const Wt::WString &label, Wt::WContainerWidget *parent = 0);
  ~FilterByObservedWidget();
  boost::logic::tribool value() const;
  void resetDefaultValue();
  Wt::Signal<> &changed() const;

private:
  D_PTR;
};

#endif // FILTERBYOBSERVEDWIDGET_H


