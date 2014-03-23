/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  Marco Gulino <email>
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

#ifndef FILTERBYCONSTELLATION_P_H
#define FILTERBYCONSTELLATION_P_H
#include "widgets/filterbyconstellation.h"

class FilterByConstellation::Private
{
  public:
    Private( FilterByConstellation *q );
    Wt::Signal<> changed;
    ConstellationFinder::Constellation selected;
  private:
    class FilterByConstellation *const q;
};
#endif // FILTERBYCONSTELLATION_P_H
