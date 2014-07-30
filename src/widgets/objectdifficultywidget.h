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

#ifndef OBJECTDIFFICULTYWIDGET_H
#define OBJECTDIFFICULTYWIDGET_H

#include <Wt/WContainerWidget>
#include "utils/d_ptr.h"

struct NgcObject;
class Telescope;
class ObjectDifficultyWidget : public Wt::WContainerWidget
{
  public:
    ~ObjectDifficultyWidget();
    ObjectDifficultyWidget( const Wt::Dbo::ptr< NgcObject > &object, const Wt::Dbo::ptr< Telescope > &telescope, double maxAltitudeInDegrees, Wt::WContainerWidget *parent = 0 );
    bool hasDifficulty() const;
private:
    D_PTR;
};

#endif // OBJECTDIFFICULTYWIDGET_H
