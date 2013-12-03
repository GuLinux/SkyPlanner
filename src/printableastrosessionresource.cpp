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
#include <Wt/WText>
#include <Wt/Utils>
#include "ephemeris.h"
#include "utils/format.h"
#include "widgets/objectnameswidget.h"
#include "constellationfinder.h"
#include "widgets/objectdifficultywidget.h"
#include <hpdf.h>
#include <Wt/Render/WPdfRenderer>

using namespace Wt;
using namespace std;

PrintableAstroSessionResource::Private::Private(const Dbo::ptr< AstroSession >& astroSession, Session& session, PrintableAstroSessionResource* q) 
  : astroSession(astroSession), session(session), rowsSpacing(0), q(q)
{
}

PrintableAstroSessionResource::PrintableAstroSessionResource(const Dbo::ptr<AstroSession> &astroSession, Session &session, WObject* parent)
  : WResource(parent), d(astroSession, session, this)
{
}

PrintableAstroSessionResource::~PrintableAstroSessionResource()
{
}

void PrintableAstroSessionResource::setTelescope(const Dbo::ptr<Telescope> &telescope)
{
  d->telescope = telescope;
}
void PrintableAstroSessionResource::setRowsSpacing(int spacing)
{
  d->rowsSpacing = spacing;
}

void PrintableAstroSessionResource::setReportType(ReportType type)
{
  d->reportType = type;
}

namespace {
    void error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no,
		       void *user_data) {
	fprintf(stderr, "libharu error: error_no=%04X, detail_no=%d\n",
		(unsigned int) error_no, (int) detail_no);
    }
}

void PrintableAstroSessionResource::handleRequest(const Wt::Http::Request &request, Wt::Http::Response &response) {
  WTemplate printable;
  Dbo::Transaction t(d->session);
  printable.setTemplateText("<html>\
    <head>\
      <title>${title}</title>\
      <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\
      <style type=\"text/css\">\
      @media print {\
	table { page-break-inside:auto }\
	tr    { page-break-inside:avoid; page-break-after:auto }\
	td    { page-break-inside:avoid; page-break-after:auto }\
      }\
      </style>\
    </head> \
  <body>\
  <center><h2>${title}</h2></center>\
  ${<have-place>}\
  <p>${sessionDate}, Moon Phase: ${moonPhase}%</p>\
  <p>Sun: rising at ${sunRise}, setting at ${sunSet}</p>\
  <p>Moon: rising at ${moonRise}, setting at ${moonSet}</p>\
  ${<have-telescope>}<p>Suggestions for telescope: \"${telescope-name}\", diameter ${telescope-diameter}mm, focal length ${telescope-focal-length}mm</p>${</have-telescope>}\
  ${</have-place>}\
  <table border=\"1\">\
    <tr>\
      <th>Names</th>\
      <th>AR</th>\
      <th>Dec</th>\
      <th>Constellation</th>\
      <th>Size</th>\
      <th>Magn.</th>\
      <th>Type</th>\
      ${<have-place>}\
      <th>Highest</th>\
      <th>Max</th>\
      ${</have-place>}\
      ${<have-telescope>}\
      <th>Difficulty</th>\
      ${</have-telescope>}\
    </tr>\
    ${table-rows}\
  </table>\
  </body></html>\
  ", XHTMLUnsafeText);
  printable.bindString("title", d->astroSession->name());
  printable.setCondition("have-place", d->astroSession->position());
  printable.setCondition("have-telescope", d->telescope);
  if(d->telescope) {
    printable.bindString("telescope-name", d->telescope->name());
    printable.bindInt("telescope-diameter", d->telescope->diameter());
    printable.bindInt("telescope-focal-length", d->telescope->focalLength());
  }
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
  auto sessionObjectsDbCollection = d->astroSession->astroSessionObjects();
  vector<dbo::ptr<AstroSessionObject>> sessionObjects(sessionObjectsDbCollection.begin(), sessionObjectsDbCollection.end());
  sort(begin(sessionObjects), end(sessionObjects), [&](const dbo::ptr<AstroSessionObject> &a, const dbo::ptr<AstroSessionObject> &b){
    return a->bestAltitude(ephemeris, -3 ).when < b->bestAltitude(ephemeris, -3).when;
  });
  for(auto sessionObject: sessionObjects) {
    WTemplate rowTemplate;
    rowTemplate.setTemplateText("<tr>\
    <td>${namesWidget}</td>\
    <td>${ar}</td>\
    <td>${dec}</td>\
    <td>${constellation}</td>\
    <td>${size}</td>\
    <td>${magnitude}</td>\
    <td>${type}</td>\
    ${<have-place>}\
    <td>${highestAt}</td>\
    <td>${maxAltitude}</td>\
    ${</have-place>}\
    ${<have-telescope>}\
    <td>${difficulty}</td>\
    ${</have-telescope>}\
    </tr>\
    ${<have-description>}\
    <tr><td colspan=\"${total-columns}\">${description}</td></tr>\
    ${</have-description>}\
    ${<have-rows-spacing>}\
    <tr><td colspan=\"${total-columns}\" style=\"height: ${rows-spacing}em\" /></tr>\
    ${</have-rows-spacing>}\
    \n", XHTMLUnsafeText);
    rowTemplate.bindWidget("namesWidget", new ObjectNamesWidget{sessionObject->ngcObject(), d->session, d->astroSession, ObjectNamesWidget::Printable});
    rowTemplate.bindString("ar", Utils::htmlEncode( sessionObject->coordinates().rightAscension.printable(Angle::Hourly) ));
    rowTemplate.bindWidget("dec", new WText{Utils::htmlEncode( WString::fromUTF8( sessionObject->coordinates().declination.printable() )) } );
    rowTemplate.bindString("constellation", ConstellationFinder::getName(sessionObject->coordinates()).name);
    rowTemplate.bindString("size", Utils::htmlEncode( WString::fromUTF8( Angle::degrees(sessionObject->ngcObject()->angularSize()).printable() )));
    rowTemplate.bindString("magnitude", format("%.1f") % sessionObject->ngcObject()->magnitude() );
    rowTemplate.bindString("type", sessionObject->ngcObject()->typeDescription());
    rowTemplate.setCondition("have-place", d->astroSession->position());
    rowTemplate.setCondition("have-telescope", d->telescope);
    rowTemplate.bindInt("total-columns", d->astroSession->position() ? 10 : 8);
    auto bestAltitude = sessionObject->bestAltitude(ephemeris, 1);
    if(d->astroSession->position()) {
      rowTemplate.bindString("highestAt", WDateTime::fromPosixTime( bestAltitude.when).time().toString() );
      rowTemplate.bindString("maxAltitude", Utils::htmlEncode(WString::fromUTF8(bestAltitude.coordinates.altitude.printable() )) );
    }
    rowTemplate.bindWidget("difficulty", new ObjectDifficultyWidget{sessionObject->ngcObject(), d->telescope, bestAltitude.coordinates.altitude.degrees() });
    rowTemplate.setCondition("have-description",  !sessionObject->description().empty());
    rowTemplate.bindString("description", Utils::htmlEncode(WString::fromUTF8(sessionObject->description())));
    rowTemplate.setCondition("have-rows-spacing", d->rowsSpacing > 0);
    rowTemplate.bindString("rows-spacing", format("%.1f") % (static_cast<double>(d->rowsSpacing) * 1.3) );
    rowTemplate.renderTemplate(tableRows);
  }
  printable.bindString("table-rows", WString::fromUTF8(tableRows.str()));
  // d->reportType = PDF;
  if(d->reportType == HTML) {
    printable.renderTemplate(response.out());
    return;
  }
  suggestFileName(format("%s.pdf") % d->astroSession->name());
  response.setMimeType("application/pdf");
  HPDF_Doc pdf = HPDF_New(error_handler, 0);
  //HPDF_UseUTFEncodings(pdf);
  
  HPDF_Page page = HPDF_AddPage(pdf);
  HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);

  Wt::Render::WPdfRenderer renderer(pdf, page);
  renderer.setMargin(2.54);
  renderer.setDpi(96);
  stringstream buffer;
  printable.renderTemplate(buffer);
  renderer.render(WString::fromUTF8(buffer.str()));
  
  HPDF_SaveToStream(pdf);
  unsigned int size = HPDF_GetStreamSize(pdf);
  HPDF_BYTE *buf = new HPDF_BYTE[size];
  HPDF_ReadFromStream (pdf, buf, &size);
  HPDF_Free(pdf);
  response.out().write((char*)buf, size);
  delete[] buf;
}