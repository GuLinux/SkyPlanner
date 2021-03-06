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

#ifndef FILTERBYCATALOGUE_P_H
#define FILTERBYCATALOGUE_P_H
#include "widgets/filters/filterbyconstellation.h"
#include "session.h"

class FilterByCatalogue::Private
{
  public:
    Private( Session &session, FilterByCatalogue *q );
    Session &session;
    Wt::Signal<> changed;
    Wt::WStandardItemModel *model;
    Wt::WComboBox *cataloguesCombo;
  private:
    class FilterByCatalogue *const q;
};
#endif // FILTERBYCATALOGUE_P_H
