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
#ifndef ASTROSESSIONSLISTTAB_H
#define ASTROSESSIONSLISTTAB_H

#include <Wt/WContainerWidget>
#include "c++/dptr.h"
#include "ptr_defs.h"

class AstroSession;
class Session;
class AstroSessionsListTab : public Wt::WContainerWidget
{
public:
    ~AstroSessionsListTab();
    AstroSessionsListTab(Session &session, Wt::WContainerWidget* parent = 0);
    Wt::Signal<AstroSessionPtr> &deletingSession() const;
    Wt::Signal<AstroSessionPtr> &sessionClicked() const;
    void reload();
private:
    D_PTR;
};

#endif // ASTROSESSIONSLISTTAB_H
