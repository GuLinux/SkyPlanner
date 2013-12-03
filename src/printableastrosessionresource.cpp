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
#include <Wt/WTemplate>
#include "ephemeris.h"
#include "utils/format.h"

using namespace Wt;
using namespace std;

PrintableAstroSessionResource::Private::Private(const Dbo::ptr< AstroSession >& astroSession, Session& session, PrintableAstroSessionResource* q) 
  : astroSession(astroSession), session(session), q(q)
{
}

PrintableAstroSessionResource::PrintableAstroSessionResource(const Dbo::ptr<AstroSession> &astroSession, Session &session, WObject* parent)
  : WResource(parent), d(astroSession, session, this)
{
  setInternalPath("astrosession_printable");
}

PrintableAstroSessionResource::~PrintableAstroSessionResource()
{
}

void PrintableAstroSessionResource::handleRequest(const Wt::Http::Request &request, Wt::Http::Response &response) {
  WTemplate printable;
  Dbo::Transaction t(d->session);
  printable.setTemplateText("<html><head><title>${title}</title></head> \
  <body>\
  <center><h2>${title}</h2></center>\
  ${<have-place>}\
  <p>${sessionDate}, Moon Phase: ${moonPhase}%</p>\
  <p>Sun: rising at ${sunRise}, setting at ${sunSet}</p>\
  <p>Moon: rising at ${moonRise}, setting at ${moonSet}</p>\
  ${</have-place>}\
  <table>\
    <tr>\
      <th>Names</th>\
      <th>AR</th>\
      <th>Dec</th>\
      <th>Dec</th>\
      <th>Constellation</th>\
      <th>Size</th>\
      <th>Magn.</th>\
      <th>Type</th>\
      ${<have-place>}\
      <th>Highest</th>\
      <th>Max</th>\
      ${</have-place>}\
      <th>Difficulty</th>\
    </tr>\
    ${table-rows}\
  </table>\
  </body></html>\
  ", XHTMLUnsafeText);
  printable.bindString("title", d->astroSession->name());
  printable.setCondition("have-place", d->astroSession->position());
  Ephemeris ephemeris(d->astroSession->position());
  printable.bindInt("moonPhase", static_cast<int>(ephemeris.moonPhase(d->astroSession->when()).illuminated_fraction*100.));
  printable.bindString("sessionDate", d->astroSession->wDateWhen().date().toString("dddd dd MMMM yyyy"));
  if(d->astroSession->position()) {
    auto sun = ephemeris.sun(d->astroSession->when());
    auto moon = ephemeris.moon(d->astroSession->when());
    auto formatTime = [](const boost::posix_time::ptime &t) { return (format("%02d:%02d") % t.time_of_day().hours() % t.time_of_day().minutes()).str(); };
    printable.bindString("sunRise",formatTime(sun.rise));
    printable.bindString("sunSet", formatTime(sun.set));
    printable.bindString("moonRise", formatTime(moon.rise));
    printable.bindString("moonSet", formatTime(moon.set));
  }
  stringstream tableRows;
  printable.bindString("table-rows", tableRows.str());
  printable.renderTemplate(response.out());
}
