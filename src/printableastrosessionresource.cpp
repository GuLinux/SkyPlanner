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

#include "printableastrosessionresource.h"
#include "private/printableastrosessionresource_p.h"
#include "utils/d_ptr_implementation.h"
#include "Models"
#include "session.h"
#include <Wt/Http/Response>

using namespace Wt;
using namespace std;

PrintableAstroSessionResource::Private::Private(const Dbo::ptr< AstroSession >& astroSession, Session& session, PrintableAstroSessionResource* q) 
  : astroSession(astroSession), session(session), q(q)
{
}

PrintableAstroSessionResource::PrintableAstroSessionResource(const Dbo::ptr<AstroSession> &astroSession, Session &session, WObject* parent)
  : WResource(parent), d(astroSession, session, this)
{
}

PrintableAstroSessionResource::~PrintableAstroSessionResource()
{
}

void PrintableAstroSessionResource::handleRequest(const Wt::Http::Request &request, Wt::Http::Response &response) {
  response.out() << "Printable astro session test";
}
