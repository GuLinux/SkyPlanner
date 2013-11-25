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

#ifndef ASTROSESSIONSLISTTAB_P_H
#define ASTROSESSIONSLISTTAB_P_H
#include "astrosessionslisttab.h"

namespace Wt {
class WTable;
}

class Session;
class AstroSessionsListTab::Private
{
public:
    Private(Session& session, AstroSessionsListTab* q);
    Session &session;
    Wt::WTable *sessionsTable;
    void populateSessions();
private:
    class AstroSessionsListTab* const q;
};
#endif // ASTROSESSIONSLISTTAB_P_H
