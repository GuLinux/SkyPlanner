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
#ifndef ACTIVESESSIONSRESOURCE_P_H
#define ACTIVESESSIONSRESOURCE_P_H

#include "activesessionsresource.h"
#include "skyplanner.h"

class ActiveSessionsResource::Private
{
public:
    Private(std::vector<SkyPlanner*> &sessions, const std::string &password, ActiveSessionsResource *q);
    std::vector<SkyPlanner*> &sessions;
    const std::string password;
private:
    ActiveSessionsResource *q;
};

#endif // ACTIVESESSIONSRESOURCE_H


