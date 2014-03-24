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

#ifndef PRINTABLEASTROSESSIONRESOURCE_P_H
#define PRINTABLEASTROSESSIONRESOURCE_P_H
#include "exportastrosessionresource.h"

#include <Wt/Dbo/ptr>
class AstroSession;
class Session;
class ExportAstroSessionResource::Private
{
public:
    Private(const Wt::Dbo::ptr<AstroSession> &astroSession, Session &session, Timezone timezone, ExportAstroSessionResource* q);
    Wt::Dbo::ptr<AstroSession> astroSession;
    Session &session;
    Timezone timezone;
    Wt::Dbo::ptr<Telescope> telescope;
    int rowsSpacing;
    ReportType reportType = HTML;
    double fontScale = 1.0;
    int namesLimit = 0;
private:
    class ExportAstroSessionResource* const q;
};
#endif // PRINTABLEASTROSESSIONRESOURCE_P_H
