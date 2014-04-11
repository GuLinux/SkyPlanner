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

#ifndef ASTROOBJECTSTABLE_P_H
#define ASTROOBJECTSTABLE_P_H
#include "widgets/astroobjectstable.h"
#include "session.h"
#include <Wt/WTable>

class AstroObjectsTable::Private
{
  public:
    Private(Session &session, const std::vector<AstroObjectsTable::Action> &actions, AstroObjectsTable *q );
    Session &session;
    std::vector<AstroObjectsTable::Action> actions; 
    Wt::WTable *objectsTable;
    Wt::WTableRow *selectedRow = nullptr;
    void header();
  private:
    class AstroObjectsTable *const q; 
};
#endif

