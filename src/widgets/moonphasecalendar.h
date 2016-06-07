/*
 * Copyright (C) 2016  <copyright holder> <email>
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

#ifndef MOONPHASECALENDAR_H
#define MOONPHASECALENDAR_H

#include "c++/dptr.h"
#include <Wt/WCalendar>
#include <Wt/WString>

class MoonPhaseCalendar : public Wt::WCalendar
{
public:
    MoonPhaseCalendar(Wt::WContainerWidget* parent = nullptr);
    ~MoonPhaseCalendar();
    
    virtual WWidget* renderCell(WWidget* widget, const Wt::WDate& date);

    static Wt::WDialog *dialog(Wt::WObject *parent = nullptr);
    static Wt::WPushButton *button(Wt::WContainerWidget* parent = nullptr, const Wt::WString& text = {});
private:
    D_PTR
};

#endif // MOONPHASECALENDAR_H
