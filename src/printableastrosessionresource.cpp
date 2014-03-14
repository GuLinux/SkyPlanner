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
#include "widgets/cataloguesdescriptionwidget.h"
#include "constellationfinder.h"
#include "widgets/objectdifficultywidget.h"
#include <Wt/Render/WPdfRenderer>
#include <Wt/WServer>

#ifndef DISABLE_LIBHARU
#include <hpdf.h>
#if HPDF_MAJOR_VERSION >= 2
#if HPDF_MINOR_VERSION >= 3
#define HAVE_LIBHARU_UTF8
#endif
#endif

#ifndef HAVE_LIBHARU_UTF8
#warning "Using libharu < 2.3, without UTF-8 support, this might lead to undefined results"
#endif
#endif

using namespace Wt;
using namespace std;

PrintableAstroSessionResource::Private::Private(const Dbo::ptr< AstroSession >& astroSession, Session& session, Timezone timezone, PrintableAstroSessionResource* q)
  : astroSession(astroSession), session(session), rowsSpacing(0), timezone(timezone), q(q)
{
}

PrintableAstroSessionResource::PrintableAstroSessionResource(const Dbo::ptr<AstroSession> &astroSession, Session &session, Timezone timezone, WObject* parent)
  : WResource(parent), d(astroSession, session, timezone, this)
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

void PrintableAstroSessionResource::setFontScale(double fontScale)
{
  d->fontScale = fontScale;
}

#ifndef DISABLE_LIBHARU
namespace {
    void error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void *user_data) {
      WServer::instance()->log("error") << (format("libharu error: error_no=%04X, detail_no=%d\n") % (unsigned int) error_no % (int) detail_no).str();
    }
}
#endif
void PrintableAstroSessionResource::handleRequest(const Wt::Http::Request &request, Wt::Http::Response &response) {
  WTemplate printable;
  Dbo::Transaction t(d->session);
  printable.setTemplateText("\
  ${<render-type-html>}\
  <html>\n\
    <head>\n\
      <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\
      <title>${title}</title>\n\
      <style type=\"text/css\">\n\
      td { font-size: 0.7em; }\n\
      th { font-size: 0.8em; }\n\
      @media print {\
	table { page-break-inside:auto }\
	tr    { page-break-inside:avoid; page-break-after:auto }\
	td    { page-break-inside:avoid; page-break-after:auto; }\
      }\
      </style>\n\
    </head>\
  <body>\n\
  ${</render-type-html>}\
  ${<render-type-pdf>}\
      <style type=\"text/css\">\n\
      	table { page-break-inside:auto }\
	tr    { page-break-inside:avoid; page-break-after:auto }\
      </style>\n\
  ${</render-type-pdf>}\
  <h2 style=\"text-align: center; \">${title}</h2>\n\
  <p>Time in ${timezone_info}</p>\n\
  ${<have-place>}\
  <p>${sessionDate}, Moon Phase: ${moonPhase}%, ${objects-number} objects.</p>\n\
  <div>\
  Sun: rising at ${sunRise}, setting at ${sunSet}<br />\n\
  Astronomical Twilight: begins at ${astroTwilightBegin}, ends at ${astroTwilightEnd}<br />\n\
  Moon: rising at ${moonRise}, setting at ${moonSet}</div>\n\
  ${<have-telescope>}<p>Suggestions for telescope: \"${telescope-name}\", diameter ${telescope-diameter}mm, focal length ${telescope-focal-length}mm</p>${</have-telescope>}\
  ${</have-place>}\
  <table border=\"1\">\n\
    <thead>\n\
      <tr>\n\
	<th>Names</th>\n\
	<th>AR</th>\n\
	<th>Dec</th>\n\
	<th>Constellation</th>\n\
	<th>Size</th>\n\
	<th>Magn.</th>\n\
	<th>Type</th>\n\
	${<have-place>}\
	<th>Highest</th>\n\
	<th>Max</th>\n\
	${</have-place>}\
	${<have-telescope>}\
	<th>Difficulty</th>\n\
	${</have-telescope>}\
      </tr>\n\
    </thead>\n\
    <tbody>\n\
      ${table-rows}\
    </tbody>\n\
  </table>\n\
  ${<render-type-html>}\
  </body></html>\n\
  ${</render-type-html>}\
  ", XHTMLUnsafeText);
  printable.setCondition("render-type-html", d->reportType == HTML);
  printable.setCondition("render-type-pdf", d->reportType == PDF);
  printable.bindString("title", WString::fromUTF8(d->astroSession->name()));
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
  printable.bindString("timezone_info", d->timezone.timeZoneName);
  if(d->astroSession->position()) {
    auto sun = ephemeris.sun(d->astroSession->when());
    auto twilight = ephemeris.sunAstronomical(d->astroSession->when());
    auto moon = ephemeris.moon(d->astroSession->when());
    auto formatTime = [=](const boost::posix_time::ptime &time) { auto t = d->timezone.fix(time); return (format("%02d:%02d") % t.time_of_day().hours() % t.time_of_day().minutes()).str(); };
    printable.bindString("sunRise",formatTime(sun.rise));
    printable.bindString("sunSet", formatTime(sun.set));
    printable.bindString("moonRise", formatTime(moon.rise));
    printable.bindString("moonSet", formatTime(moon.set));
    printable.bindString("astroTwilightBegin", formatTime(twilight.rise));
    printable.bindString("astroTwilightEnd", formatTime(twilight.set));
  }
  stringstream tableRows;
  auto sessionObjectsDbCollection = d->astroSession->astroSessionObjects();
  vector<dbo::ptr<AstroSessionObject>> sessionObjects(sessionObjectsDbCollection.begin(), sessionObjectsDbCollection.end());
  sort(begin(sessionObjects), end(sessionObjects), [&](const dbo::ptr<AstroSessionObject> &a, const dbo::ptr<AstroSessionObject> &b){
    return a->bestAltitude(ephemeris, -3 ).when < b->bestAltitude(ephemeris, -3).when;
  });
  printable.bindInt("objects-number", sessionObjects.size());
  for(auto sessionObject: sessionObjects) {
    WTemplate rowTemplate;
    rowTemplate.setTemplateText("<tr style=\"page-break-inside:avoid; page-break-after:auto\">\n\
    <td>${namesWidget}</td>\n\
    <td style='white-space:nowrap; font-size: smaller;'>${ar}</td>\n\
    <td style='white-space:nowrap; font-size: smaller;'>${dec}</td>\n\
    <td>${constellation}</td>\n\
    <td style='white-space:nowrap; font-size: smaller;'>${size}</td>\n\
    <td>${magnitude}</td>\n\
    <td>${type}</td>\n\
    ${<have-place>}\
    <td style='white-space:nowrap; font-size: smaller;'>${highestAt}</td>\n\
    <td style='white-space:nowrap; font-size: smaller;'>${maxAltitude}</td>\n\
    ${</have-place>}\
    ${<have-telescope>}\
    <td>${difficulty}</td>\n\
    ${</have-telescope>}\
    </tr>\n\
    ${<have-catalogues-description>}\
    <tr><td colspan=\"${total-columns}\">${catalogues-description}</td></tr>\n\
    ${</have-catalogues-description>}\
    ${<have-description>}\
    <tr><td colspan=\"${total-columns}\">${description}</td></tr>\n\
    ${</have-description>}\
    ${<have-rows-spacing>}\
    <tr style=\"page-break-inside:avoid; page-break-after:auto\">\
      <td colspan=\"${total-columns}\" style=\"height: ${rows-spacing}em; page-break-inside:avoid; page-break-after:auto\" />\
    </tr>\n\
    ${</have-rows-spacing>}\
    \n", XHTMLUnsafeText);
    rowTemplate.bindWidget("namesWidget", new ObjectNamesWidget{sessionObject->ngcObject(), d->session, d->astroSession, ObjectNamesWidget::Printable});
    rowTemplate.bindString("ar", sessionObject->coordinates().rightAscension.printable(Angle::Hourly));
    rowTemplate.bindWidget("dec", new WText{WString::fromUTF8( sessionObject->coordinates().declination.printable(Angle::Degrees, Angle::HTML) ) } );
    rowTemplate.bindString("constellation", ConstellationFinder::getName(sessionObject->coordinates()).name);
    rowTemplate.bindString("size", WString::fromUTF8( Angle::degrees(sessionObject->ngcObject()->angularSize()).printable(Angle::Degrees, Angle::HTML) ));
    rowTemplate.bindString("magnitude", format("%.1f") % sessionObject->ngcObject()->magnitude() );
    rowTemplate.bindString("type", sessionObject->ngcObject()->typeDescription());
    rowTemplate.setCondition("have-place", d->astroSession->position());
    rowTemplate.setCondition("have-telescope", d->telescope);
    rowTemplate.bindInt("total-columns", d->astroSession->position() ? 10 : 8);
    auto bestAltitude = sessionObject->bestAltitude(ephemeris, 1);
    if(d->astroSession->position()) {
      rowTemplate.bindString("highestAt", WDateTime::fromPosixTime( bestAltitude.when).time().toString() );
      rowTemplate.bindString("maxAltitude", WString::fromUTF8(bestAltitude.coordinates.altitude.printable(Angle::Degrees, Angle::HTML)) );
    }
    rowTemplate.bindWidget("difficulty", new ObjectDifficultyWidget{sessionObject->ngcObject(), d->telescope, bestAltitude.coordinates.altitude.degrees() });
    rowTemplate.setCondition("have-description",  !sessionObject->description().empty());
    rowTemplate.bindString("description", Utils::htmlEncode(WString::fromUTF8(sessionObject->description())));
    rowTemplate.setCondition("have-rows-spacing", d->rowsSpacing > 0);
    rowTemplate.bindString("rows-spacing", format("%.1f") % (static_cast<double>(d->rowsSpacing) * 1.3) );


    auto dbDescriptions = sessionObject->ngcObject()->descriptions();
    rowTemplate.setCondition("have-catalogues-description", !dbDescriptions.empty());
    if(!dbDescriptions.empty()) {
      rowTemplate.bindWidget("catalogues-description", new CataloguesDescriptionWidget{dbDescriptions});
    }

    rowTemplate.renderTemplate(tableRows);
  }
  printable.bindString("table-rows", WString::fromUTF8(tableRows.str()));
  if(d->reportType == HTML) {
    printable.renderTemplate(response.out());
    return;
  }
#ifndef DISABLE_LIBHARU
  suggestFileName(format("%s.pdf") % d->astroSession->name());
  response.setMimeType("application/pdf");
  HPDF_Doc pdf = HPDF_New(error_handler, 0);
#ifdef HAVE_LIBHARU_UTF8
  HPDF_UseUTFEncodings(pdf);
  HPDF_SetCurrentEncoder(pdf, "UTF-8");
#endif
  HPDF_Page page = HPDF_AddPage(pdf);
  HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_LANDSCAPE);
  Wt::Render::WPdfRenderer renderer(pdf, page);
  renderer.setMargin(1.2);
  renderer.setDpi(96);
  renderer.setFontScale(d->fontScale);
  renderer.addFontCollection("/usr/share/fonts", true);
  stringstream buffer;
  printable.renderTemplate(buffer);
  string b = buffer.str();
  boost::replace_all(b, "°", "&deg;");
//   cerr << "--------------------" << endl << b << endl << "----------------------" << endl;
  renderer.render(WString::fromUTF8(b));
  
  HPDF_SaveToStream(pdf);
  unsigned int size = HPDF_GetStreamSize(pdf);
  HPDF_BYTE *buf = new HPDF_BYTE[size];
  HPDF_ReadFromStream (pdf, buf, &size);
  HPDF_Free(pdf);
  response.out().write((char*)buf, size);
  delete[] buf;
#endif
}
