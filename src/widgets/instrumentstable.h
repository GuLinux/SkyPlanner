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

#ifndef INSTRUMENTSTABLE_H
#define INSTRUMENTSTABLE_H

#include <Wt/WCompositeWidget>
#include <ptr_defs.h>

class Session;

class InstrumentsTable : public Wt::WCompositeWidget
{
public:
    ~InstrumentsTable();
    InstrumentsTable(const UserPtr &user, Session &session, Wt::WContainerWidget* parent = 0);
    void reload();
private:
  Session &session;
  Wt::WTable *instrumentsTable;
  UserPtr user;
};

#endif // INSTRUMENTSTABLE_H
