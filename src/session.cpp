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

#include "session.h"
#include "private/session_p.h"
#include <Wt/Dbo/backend/Sqlite3>
#include "ngcobject.h"
#include "nebuladenomination.h"
#include "utils/d_ptr_implementation.h"

using namespace std;
using namespace Wt;

Session::Private::Private() {
}

Session::Session()
{
  d->connection = make_shared<Dbo::backend::Sqlite3>("ngc.sqlite");
  setConnection(*d->connection);
  d->connection->setProperty("show-queries", "true");
  mapClass<NgcObject>("objects");
  mapClass<NebulaDenomination>("denominations");
}

Session::~Session()
{

}
