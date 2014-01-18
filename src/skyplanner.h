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

#ifndef ASTROPLANNER_H
#define ASTROPLANNER_H

#include <Wt/WApplication>
#include "utils/d_ptr.h"

class SkyPlanner : public Wt::WApplication
{
public:
    ~SkyPlanner();
    SkyPlanner(const Wt::WEnvironment& environment);
    static SkyPlanner *instance();
    enum NotificationType { Alert, Error, Success, Information };
    Wt::WContainerWidget * notification( const Wt::WString &title, const Wt::WString &content, SkyPlanner::NotificationType type, int autoHideSeconds = 0 , Wt::WContainerWidget *addTo = nullptr);
    static const std::string HOME_PATH;
private:
    D_PTR;
};

#endif // ASTROPLANNER_H
