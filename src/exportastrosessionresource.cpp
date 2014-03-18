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

#include "exportastrosessionresource.h"
#include "private/exportastrosessionresource_p.h"
#include "utils/d_ptr_implementation.h"
#include "Models"
#include "session.h"
#include <Wt/Http/Response>
#include <Wt/WTemplate>
#include <Wt/WText>
#include <Wt/Utils>
#include "ephemeris.h"
#include "utils/format.h"
#include "utils/utils.h"
#include "widgets/objectnameswidget.h"
#include "widgets/cataloguesdescriptionwidget.h"
#include "constellationfinder.h"
#include "widgets/objectdifficultywidget.h"
#include <Wt/Render/WPdfRenderer>
#include <Wt/WServer>
#include <boost/algorithm/string.hpp>

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

ExportAstroSessionResource::Private::Private(const Dbo::ptr< AstroSession >& astroSession, Session& session, Timezone timezone, ExportAstroSessionResource* q)
  : astroSession(astroSession), session(session), rowsSpacing(0), timezone(timezone), q(q)
{
}

ExportAstroSessionResource::ExportAstroSessionResource(const Dbo::ptr<AstroSession> &astroSession, Session &session, Timezone timezone, WObject* parent)
  : WResource(parent), d(astroSession, session, timezone, this)
{
}

ExportAstroSessionResource::~ExportAstroSessionResource()
{
}

void ExportAstroSessionResource::setTelescope(const Dbo::ptr<Telescope> &telescope)
{
  d->telescope = telescope;
}
void ExportAstroSessionResource::setRowsSpacing(int spacing)
{
  d->rowsSpacing = spacing;
}

void ExportAstroSessionResource::setReportType(ReportType type)
{
  d->reportType = type;
}

void ExportAstroSessionResource::setFontScale(double fontScale)
{
  d->fontScale = fontScale;
}

void ExportAstroSessionResource::setTimezone(const Timezone &timezone)
{
  d->timezone = timezone;
}

#ifndef DISABLE_LIBHARU
namespace {
    void error_handler(HPDF_STATUS error_no, HPDF_STATUS detail_no, void *user_data) {
      WServer::instance()->log("error") << (format("libharu error: error_no=%04X, detail_no=%d\n") % (unsigned int) error_no % (int) detail_no).str();
    }
}
#endif
void ExportAstroSessionResource::handleRequest(const Wt::Http::Request &request, Wt::Http::Response &response) {
  Dbo::Transaction t(d->session);
  Ephemeris ephemeris(d->astroSession->position());
  auto sessionObjectsDbCollection = d->astroSession->astroSessionObjects();
  vector<AstroSessionObjectPtr> sessionObjects(sessionObjectsDbCollection.begin(), sessionObjectsDbCollection.end());
  sort(begin(sessionObjects), end(sessionObjects), [&](const dbo::ptr<AstroSessionObject> &a, const dbo::ptr<AstroSessionObject> &b){
    return a->bestAltitude(ephemeris, -3 ).when < b->bestAltitude(ephemeris, -3).when;
  });

  if(d->reportType == CSV) {
    suggestFileName(format("%s.csv") % d->astroSession->name());
    response.setMimeType("text/csv");
    vector<string> headers{"object_column_names", "object_column_ar", "object_column_dec", "object_column_constellation", "object_column_angular_size", "object_column_magnitude", "object_column_type"};
    if(d->astroSession->position()) {
      headers.push_back("object_column_highest_time");
      headers.push_back("object_column_max_altitude");
    }
    transform(begin(headers), end(headers), begin(headers), [](const std::string &s) { return WString::tr(s).toUTF8(); });
    transform(begin(headers), end(headers), begin(headers), bind(::Utils::csv, placeholders::_1, ','));
    response.out() << boost::algorithm::join(headers, ",") << endl;

    for(AstroSessionObjectPtr object: sessionObjects) {
      vector<string> fields;
      auto add = [&fields](const string &field) { fields.push_back(::Utils::csv(field)); };
      add(boost::algorithm::join(NgcObject::namesByCatalogueImportance(t, object->ngcObject()), ", "));
      add(format("%.3f") % object->coordinates().rightAscension.hours());
      add(format("%.3f") % object->coordinates().declination.degrees());
      add(ConstellationFinder::getName(object->ngcObject()->coordinates()).name);
      add(format("%.3f") % object->ngcObject()->angularSize());
      add(format("%.3f") % object->ngcObject()->magnitude());
      add(object->ngcObject()->typeDescription().toUTF8());
      if(d->astroSession->position()) {
        auto bestAltitude = object->bestAltitude(ephemeris, 1);
        add(boost::posix_time::to_simple_string(bestAltitude.when.time_of_day()));
        add(format("%.3f") % bestAltitude.coordinates.altitude.degrees());
      }
      response.out() << boost::algorithm::join(fields, ",") << endl;
    }
    return;
  }

  WTemplate printable;
  printable.addFunction("tr", &WTemplate::Functions::tr);
  printable.setTemplateText(WString::tr("printable-session"), XHTMLUnsafeText);
  printable.setCondition("render-type-html", d->reportType == HTML);
  printable.setCondition("render-type-pdf", d->reportType == PDF);
  printable.bindString("title", WString::fromUTF8(d->astroSession->name()));
  printable.setCondition("have-place", d->astroSession->position());
  printable.setCondition("have-telescope", d->telescope);
  if(d->telescope) {
    printable.bindString("printable_telescope_info", WString::tr("printable_telescope_info")
                         .arg(d->telescope->name())
                         .arg(d->telescope->diameter())
                         .arg(d->telescope->focalLength())
                         );
  }

  printable.bindString("moonPhase", WString::tr("astrosessiontab_moon_phase").arg(static_cast<int>(ephemeris.moonPhase(d->astroSession->when()).illuminated_fraction*100.)));
  printable.bindString("sessionDate", d->astroSession->wDateWhen().date().toString("dddd dd MMMM yyyy"));
  printable.bindString("timezone_info", WString::tr("printable_timezone_info").arg(d->timezone.timeZoneName));
  if(d->astroSession->position()) {
    auto sun = ephemeris.sun(d->astroSession->when());
    auto twilight = ephemeris.astronomicalTwilight(d->astroSession->when());
    auto moon = ephemeris.moon(d->astroSession->when());
    auto darkness = ephemeris.darknessHours(d->astroSession->when() );

    auto formatTime = [=](const boost::posix_time::ptime &time) { auto t = d->timezone.fix(time); return (format("%02d:%02d") % t.time_of_day().hours() % t.time_of_day().minutes()).str(); };
    printable.bindString("astrosessiontab_sun_info", WString::tr("astrosessiontab_sun_info").arg(formatTime(sun.rise)).arg(formatTime(sun.set)));
    printable.bindString("astrosessiontab_astro_twilight_info", WString::tr("astrosessiontab_astro_twilight_info").arg(formatTime(twilight.rise)).arg(formatTime(twilight.set)));
    printable.bindString("astrosessiontab_moon_info", WString::tr("astrosessiontab_moon_info").arg(formatTime(moon.rise)).arg(formatTime(moon.set)));
    printable.bindString("astrosessiontab_darkness_hours", WString::tr("astrosessiontab_darkness_hours").arg(formatTime(darkness.begin)).arg(formatTime(darkness.end)).arg(boost::posix_time::to_simple_string(darkness.duration)));
  }
  stringstream tableRows;
  printable.bindString("objects-number", WString::tr("objects_number_label").arg(sessionObjects.size()));
  for(auto sessionObject: sessionObjects) {
    WTemplate rowTemplate;
    rowTemplate.setTemplateText(WString::tr("printable-session-row"), XHTMLUnsafeText);
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
