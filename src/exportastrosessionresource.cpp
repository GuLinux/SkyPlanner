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
#include <Wt/WServer>
#include <boost/algorithm/string.hpp>

#ifndef DISABLE_LIBHARU
#include <Wt/Render/WPdfRenderer>
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

#include "widgets/astroobjectstable.h"
#include "skyplanner.h"

using namespace Wt;
using namespace std;

ExportAstroSessionResource::Private::Private(const AstroSessionPtr &astroSession, Session& session, Timezone timezone, ExportAstroSessionResource* q)
  : astroSession(astroSession), session(session), rowsSpacing(0), timezone(timezone), q(q)
{
}

ExportAstroSessionResource::ExportAstroSessionResource(const AstroSessionPtr &astroSession, Session &session, Timezone timezone, WObject* parent)
  : WResource(parent), d(astroSession, session, timezone, this)
{
}

ExportAstroSessionResource::~ExportAstroSessionResource()
{
}

void ExportAstroSessionResource::setTelescope(const TelescopePtr &telescope)
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

void ExportAstroSessionResource::setNamesLimit(int namesLimit)
{
  d->namesLimit = namesLimit;
}

void ExportAstroSessionResource::setPlace(const GeoCoder::Place &place)
{
  d->place = place;
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
  Ephemeris ephemeris(d->astroSession->position(), d->timezone);
  auto sessionObjectsDbCollection = d->astroSession->astroSessionObjects();
  vector<AstroSessionObjectPtr> sessionObjects(sessionObjectsDbCollection.begin(), sessionObjectsDbCollection.end());
  sort(begin(sessionObjects), end(sessionObjects), [&](const AstroSessionObjectPtr &a, const AstroSessionObjectPtr &b){
    return a->bestAltitude(ephemeris, d->timezone).when < b->bestAltitude(ephemeris, d->timezone).when;
  });

  if(d->reportType == CSV) {
    setDispositionType(Attachment);
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
      auto add = [&fields, &t](const string &field) { fields.push_back(::Utils::csv(field)); };
      add(boost::algorithm::join(NgcObject::namesByCatalogueImportance(t, object->ngcObject()), ", "));
      add(format("%.3f") % object->coordinates().rightAscension.hours());
      add(format("%.3f") % object->coordinates().declination.degrees());
      add(ConstellationFinder::getName(object->ngcObject()->coordinates()).name);
      add(format("%.3f") % object->ngcObject()->angularSize());
      add(format("%.3f") % object->ngcObject()->magnitude());
      add(object->ngcObject()->typeDescription().toUTF8());
      if(d->astroSession->position()) {
        auto bestAltitude = object->bestAltitude(ephemeris, d->timezone);
        add(bestAltitude.when.str());
        add(format("%.3f") % bestAltitude.coordinates.altitude.degrees());
      }
      response.out() << boost::algorithm::join(fields, ",") << endl;
    }
    return;
  }

  if(d->reportType == CartesDuCiel) {
    suggestFileName(format("%s.txt") % d->astroSession->name() );
    response.out() << d->astroSession->name() << '\n';
    response.setMimeType("text/plain");

    for(AstroSessionObjectPtr object: sessionObjects) {
      string objectName;
      vector<string> names; 
      for(auto name: NgcObject::namesByCatalogueImportance(t, object->ngcObject())) {
        if(name.size() < 32 && objectName.empty() ) {
          objectName = name;
        }
        else if(boost::algorithm::join(names, ", ").size() + 2 + name.size() < 32)
          names.push_back(name);
      }
      string description = boost::algorithm::join(names, ", ");
      if(! object->description().empty() && object->description().size() + description.size() < 32)
       description += format("%s%s") % (description.empty() ? "" : ", ")  % object->description();
      response.out() << format("%-32s%9.5f %9.5f %-32s%-32s\n") % objectName % object->coordinates().rightAscension.degrees() % object->coordinates().declination.degrees() %objectName % description;
    }
    return;
  }

  if(d->reportType == KStars) {
    setDispositionType(Attachment);
    auto kStarsName = [](NgcObject::NebulaType t) {
      switch(t) {
	case NgcObject::RedStar:
	    return  "Star" ;
	case NgcObject::NebOc:
	    return  "Open Cluster" ;
	case NgcObject::NebGc:
	    return  "Globular Cluster" ;
	case NgcObject::NebN:
	    return  "Gaseous Nebula" ;
	case NgcObject::NebPn:
	    return  "Planetary Nebula" ;
	case NgcObject::NebGx:
	    return  "Galaxy" ;
	case NgcObject::NebIg:
	    return  "Galaxy" ;
	case NgcObject::NebGalCluster:
	    return  "Galaxy Cluster" ;
	case NgcObject::Asterism:
	    return  "Asterism" ;
	case NgcObject::NebDn:
	    return  "Dark Nebula" ;
	    /* TODO
	case MULT_STAR:
	    return  "Multiple Star" ;
	    */
	default:
	    return  "Unknown Type" ;
      }
    };
    response.setMimeType("text/xml");
    suggestFileName(format("%s.obslist") % d->astroSession->name());
    response.out() << WString(R"(<?xml version="1.0"?>
    <oal:observations xmlns:oal="http://observation.sourceforge.net/openastronomylog" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:schemaLocation="http://observation.sourceforge.net/openastronomylog oal20.xsd" version="2.0">
    <geodate>
        <name><![CDATA[{1}]]></name>
        <province><![CDATA[{2}]]></province>
        <country><![CDATA[{3}]]></country>
        <date><![CDATA[{4}]]></date>
    </geodate>
    <sites>
    <site>
      <name><![CDATA[{5}]]></name>
      <longitude unit="deg">{6}</longitude>
      <latitude unit="deg">{7}</latitude>
      <timezone>{8}</timezone>
    </site>
    </sites>
    <observers/>
    <sessions/>
    <targets>
    )").arg(d->place.city).arg(d->place.province).arg(d->place.country).arg(d->astroSession->wDateWhen().toString("ddMMyyyy") )
     .arg(d->place.city).arg(d->astroSession->position().longitude.degrees())
     .arg(d->astroSession->position().latitude.degrees())
     .arg(d->timezone.rawOffset/60) 
    .toUTF8();
    for(AstroSessionObjectPtr object: sessionObjects) {
      response.out() << WString(R"(
	  <target id="{1}" type="{2}">
            <datasource><![CDATA[SkyPlanner]]></datasource>
            <name><![CDATA[{3}]]></name>
            <position>
                <ra unit="rad">{4}</ra>
                <dec unit="rad">{5}</dec>
            </position>
            <constellation><![CDATA[{6}]]></constellation>
            <notes><![CDATA[{7}]]></notes>
        </target>
      )")
      .arg(*object->ngcObject()->objectId())
      .arg(kStarsName(object->ngcObject()->type()))
      .arg(WString::fromUTF8(boost::algorithm::join(NgcObject::namesByCatalogueImportance(t, object->ngcObject()), ", ")))
      .arg(object->coordinates().rightAscension.radians())
      .arg(object->coordinates().declination.radians())
      .arg(WString::fromUTF8(object->ngcObject()->constellation().name))
      .arg(WString::fromUTF8(object->description()))
      .toUTF8()
      ;
    };
    
    response.out() << R"(
    </targets>
    <scopes/>
    <eyepieces/>
    <lenses/>
    <filters/>
    <imagers/>
    </oal:observations>
    )";
    return;
  }
  WTemplate printable;
  printable.addFunction("tr", &WTemplate::Functions::tr);
  printable.setTemplateText(WString::tr("printable-session"), XHTMLUnsafeText);
  printable.setCondition("render-type-html", d->reportType == HTML);
  printable.setCondition("render-type-pdf", d->reportType == PDF);
  printable.bindString("title", WString::fromUTF8(d->astroSession->name()));
  printable.setCondition("have-place", d->astroSession->position());
  printable.setCondition("have-place-geocoding", !d->place.formattedAddress.empty() );
  printable.bindString("place-geocoding", d->place.formattedAddress );
  printable.setCondition("have-telescope", static_cast<bool>(d->telescope));
  if(d->telescope) {
    printable.bindString("printable_telescope_info", WString::tr("printable_telescope_info")
                         .arg(d->telescope->name())
                         .arg(d->telescope->diameter())
                         .arg(d->telescope->focalLength())
                         );
  }

  printable.setCondition("have-planets", true);
  AstroObjectsTable *planetsTable = new AstroObjectsTable(d->session, {}, AstroObjectsTable::NoFiltersButton, {}, {AstroObjectsTable::Names, AstroObjectsTable::AR, AstroObjectsTable::DEC, AstroObjectsTable::Constellation, AstroObjectsTable::Magnitude, AstroObjectsTable::AngularSize, AstroObjectsTable::ObservationTime, AstroObjectsTable::MaxAltitude});
  planetsTable->setTableAttribute("border", "1");
  vector<AstroObjectsTable::AstroObject> planets;
  for(auto planet: Ephemeris::allPlanets) {
    AstroObjectsTable::AstroObject astroObject;
    astroObject.planet = ephemeris.planet(planet, DateTime::fromLocal(d->astroSession->when(), d->timezone));
    planets.push_back(astroObject);
  }
  planetsTable->populate(planets, {}, d->timezone);
  printable.bindWidget("planets-table", planetsTable);


 

  printable.bindString("moonPhase", WString::tr("astrosessiontab_moon_phase").arg(static_cast<int>(ephemeris.moonPhase(d->astroSession->date()).illuminated_fraction*100.)));
  printable.bindString("sessionDate", d->astroSession->wDateWhen().toString("dddd dd MMMM yyyy"));
  printable.bindString("timezone_info", d->timezone ?  WString::tr("printable_timezone_info").arg(WString::fromUTF8(d->timezone.timeZoneName)) : WString());
  if(d->astroSession->position()) {
    printable.bindString("place-coordinates", WString::tr("astrosession_coordinates")
    .arg(WString::fromUTF8(d->astroSession->position().latitude.printable()) )
    .arg(WString::fromUTF8(d->astroSession->position().longitude.printable() ) )
	      );
    auto sun = ephemeris.sun(d->astroSession->date());
    auto twilight = ephemeris.astronomicalTwilight(d->astroSession->date());
    auto moon = ephemeris.moon(d->astroSession->date());
    auto darkness = ephemeris.darknessHours(d->astroSession->date() );

    printable.bindString("astrosessiontab_sun_info", WString::tr("astrosessiontab_sun_info").arg(sun.rise.str(DateTime::DateShort)).arg(sun.set.str(DateTime::DateShort)));
    printable.bindString("astrosessiontab_astro_twilight_info", WString::tr("astrosessiontab_astro_twilight_info").arg(twilight.rise.str(DateTime::DateShort)).arg(twilight.set.str(DateTime::DateShort)));
    printable.bindString("astrosessiontab_moon_info", WString::tr("astrosessiontab_moon_info").arg(moon.rise.str(DateTime::DateShort)).arg(moon.set.str(DateTime::DateShort)));
    printable.bindString("astrosessiontab_darkness_hours", WString::tr("astrosessiontab_darkness_hours").arg(darkness.begin.str(DateTime::DateShort)).arg(darkness.end.str(DateTime::DateShort)).arg(boost::posix_time::to_simple_string(darkness.duration)));
  }
  stringstream tableRows;
  printable.bindString("objects-number", WString::tr("objects_number_label").arg(sessionObjects.size()));
  for(auto sessionObject: sessionObjects) {
    WTemplate rowTemplate;
    rowTemplate.setTemplateText(WString::tr("printable-session-row"), XHTMLUnsafeText);
    rowTemplate.bindWidget("namesWidget", new ObjectNamesWidget{sessionObject->ngcObject(), d->session, nullptr , ObjectNamesWidget::Printable, d->namesLimit});
    rowTemplate.bindString("ar", sessionObject->coordinates().rightAscension.printable(Angle::Hourly));
    rowTemplate.bindWidget("dec", new WText{WString::fromUTF8( sessionObject->coordinates().declination.printable(Angle::Degrees, Angle::HTML) ) } );
    rowTemplate.bindString("constellation", WString::fromUTF8(ConstellationFinder::getName(sessionObject->coordinates()).name));
    rowTemplate.bindString("size", WString::fromUTF8( Angle::degrees(sessionObject->ngcObject()->angularSize()).printable(Angle::Degrees, Angle::HTML) ));
    rowTemplate.bindString("magnitude", format("%.1f") % sessionObject->ngcObject()->magnitude() );
    rowTemplate.bindString("type", sessionObject->ngcObject()->typeDescription());
    rowTemplate.setCondition("have-place", d->astroSession->position());
    rowTemplate.setCondition("have-telescope", static_cast<bool>(d->telescope));
    rowTemplate.bindInt("total-columns", d->astroSession->position() ? 10 : 8);
    auto bestAltitude = sessionObject->bestAltitude(ephemeris, d->timezone);
    if(d->astroSession->position()) {
      rowTemplate.bindString("highestAt", bestAltitude.when.str() );
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

  try {
    renderer.render(WString::fromUTF8(b));
    HPDF_SaveToStream(pdf);
    unsigned int size = HPDF_GetStreamSize(pdf);
    HPDF_BYTE *buf = new HPDF_BYTE[size];
    HPDF_ReadFromStream (pdf, buf, &size);
    HPDF_Free(pdf);
    response.out().write((char*)buf, size);
    delete[] buf;
  } catch(std::exception &e) {
    spLog("error") << "error rendering pdf: " << e.what();
    spLog("error") << b;
    throw e;
  }
#endif
}
