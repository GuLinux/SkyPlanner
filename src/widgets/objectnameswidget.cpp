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

#include "Models"
#include "objectnameswidget.h"
#include "Wt-Commons/wt_helpers.h"
#include "utils/format.h"
#include <Wt/WText>
#include <Wt/Utils>
#include "utils/d_ptr_implementation.h"
#include "utils/autopostresource.h"
#include <Wt/WPopupMenu>
#include <Wt/WAnchor>
#include <Wt/WJavaScript>
#include <Wt/WTemplate>
#include "session.h"
#include "types.h"

using namespace WtCommons;
using namespace Wt;
using namespace std;

class ObjectNamesWidget::Private {
public:
  Private(Session &session, ObjectNamesWidget *q) : session(session), q(q) {}
  Session &session;
private:
  ObjectNamesWidget * const q;
};

ObjectNamesWidget::ObjectNamesWidget(const Wt::Dbo::ptr<NgcObject> &object, Session &session, const Wt::Dbo::ptr<AstroSession> &astroSession, RenderType renderType, WContainerWidget *parent)
  : WContainerWidget(parent), d(session, this)
{
    auto names = object->nebulae();
    stringstream namesStream;
    string separator;
    for(auto name: names) {
      namesStream << separator << name->name();
      separator = ", ";
    }
    WString namesJoined = Utils::htmlEncode(WString::fromUTF8(namesStream.str()));
    if(renderType == Printable) {
      setInline(true);
      addWidget(new WText{namesJoined});
      return;
    }
    WAnchor *namesText = WW<WAnchor>("", namesJoined).css("link");
    addWidget(namesText);
    
    namesText->clicked().connect([=](WMouseEvent e) {
      Dbo::Transaction t(d->session);
      WPopupMenu *popup = new WPopupMenu();
      auto addLink = [=](const WString &label, const WLink &url) {
	WMenuItem *menuItem = popup->addItem(label);
        menuItem->setLink(url);
	menuItem->setLinkTarget(TargetNewWindow);
      };
      popup->addSectionHeader(WString::tr("objectnames_more_info"));
      WMenuItem *imagesMenuItem = popup->addItem("Images");
      imagesMenuItem->triggered()->connect([=](WMenuItem*, _n5) {
	WDialog *imagesDialog = new WDialog();
	imagesDialog->setTitle("Images for {1}");
	// http://archive.stsci.edu/cgi-bin/dss_search?v=phase2_gsc2&r=19+59+36.38&d=%2B22+43+15.7&e=J2000&h=15.0&w=15.0&f=gif&c=none&fov=SM97&v3=
	imagesDialog->content()->addWidget(new WImage(format("")));
	imagesDialog->footer()->addWidget(WW<WPushButton>("Close").onClick([=](WMouseEvent){ imagesDialog->accept(); });
      });
      WMenuItem *ngcIcMenuItem = popup->addItem("NGC-IC Project Page");
      ngcIcMenuItem->setLink(new AutoPostResource{"http://www.ngcicproject.org/ngcicdb.asp", {{"ngcicobject", object->objectId()}}});
      ngcIcMenuItem->setLinkTarget(TargetNewWindow);
      string catName;
      int catNumber;
      for(auto nebula: object->nebulae()) {
	if(nebula->catalogue() == "NGC" || nebula->catalogue() == "IC") {
	  catName = nebula->catalogue();
	  catNumber = nebula->number();
	}
      }
      
      stringstream dsoBrowserLink;
      dsoBrowserLink << "http://dso-browser.com/dso/info/" << catName << "/" << catNumber;
      dsoBrowserLink << "?year=" << astroSession->when().date().year() 
	<< "&month=" <<astroSession->when().date().month().as_number() 
	<< "&day=" << astroSession->when().date().day();
      if(astroSession->position()) {
	cerr << "Longitude: " << astroSession->position().longitude.degrees() << endl;
	string longitudeEmisphere = astroSession->position().longitude.degrees() > 0 ? "E" : "W" ;
	Angle::Sexagesimal longitude = astroSession->position().longitude.degrees() > 0 
	  ? astroSession->position().longitude.sexagesimal()
	  : Angle::degrees(-astroSession->position().longitude.degrees() ).sexagesimal();
	
	string latitudeEmisphere = astroSession->position().latitude.degrees() > 0 ? "N" : "S";
	Angle::Sexagesimal latitude = (astroSession->position().latitude.degrees() > 0)
	  ? astroSession->position().latitude.sexagesimal()
	  : Angle::degrees(-astroSession->position().latitude.degrees()).sexagesimal();
	dsoBrowserLink << "&lat_deg=" << latitude.degrees << "&lat_min=" << latitude.minutes << "&lat_sec=" << static_cast<int>(latitude.seconds) << "&lon_hem=" << longitudeEmisphere;
	dsoBrowserLink << "&lon_deg=" << longitude.degrees << "&lon_min=" << longitude.minutes << "&lon_sec=" << static_cast<int>(longitude.seconds) << "&lat_hem=" << latitudeEmisphere;
      // ?lat_deg=45&lat_min=29&lat_sec=31&lat_hem=N&month=12&day=2&year=2013&timezone=0&lon_deg=9&lon_min=17&lon_sec=53&lon_hem=E&min_alt=0&hour=0
      }
      addLink("DSO Browser", dsoBrowserLink.str());
      
      popup->addSectionHeader(WString::tr("objectnames_search_menu_title"));
      addLink(WString::tr("objectnames_google_search"), (format("http://www.google.com/search?q=%s%%20%s") % catName % catNumber ).str() );
      addLink(WString::tr("objectnames_google_images_search"), (format("http://www.google.com/images?q=%s%%20%s") % catName % catNumber ).str() );
      popup->popup(e);
    });
}

ObjectNamesWidget::~ObjectNamesWidget()
{
}