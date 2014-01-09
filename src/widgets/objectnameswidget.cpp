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
#include <Wt/WDialog>
#include <Wt/WPushButton>
#include <Wt/WStackedWidget>
#include <Wt/WLabel>
#include <Wt/WTimer>
#include "session.h"
#include "types.h"
#include "astroplanner.h"
#include <boost/regex.hpp>
#include "widgets/dsspage.h"

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
    auto dboDenominations = object->nebulae();
    vector<NebulaDenominationPtr> denominations{dboDenominations.begin(), dboDenominations.end()};
    vector<string> names;
    static map<string,int> catalogRatings {
      {"Messier", -99},
      {"NGC", -98},
      {"IC", -97},
      {"Arp", -96},
      {"Caldwell", -95},
      {"Abell", -94},
      {"UGC", -93},
      {"MCG", -92},
    };
    sort(denominations.begin(), denominations.end(), [](const NebulaDenominationPtr &a, const NebulaDenominationPtr &b) {
      if(!a->catalogue() && ! b->catalogue())
        return a->name() < b->name();
      if(!a->catalogue())
        return true;
      if(!b->catalogue())
        return false;
      return catalogRatings[*a->catalogue()] < catalogRatings[*b->catalogue()];
    });
    for(auto denomination: denominations) {
      if(std::count(names.begin(), names.end(), denomination->name()) == 0)
        names.push_back(denomination->name());
    }
    stringstream namesStream;
    string separator;
    for(auto name: names) {
      namesStream << separator << name;
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
      auto addLink = [=](const WString &label, const WLink &url, WMenu *menu = 0) {
        if(!menu) menu = popup;
	WMenuItem *menuItem = menu->addItem(label);
        menuItem->setLink(url);
	menuItem->setLinkTarget(TargetNewWindow);
      };
      popup->addSectionHeader(WString::tr("objectnames_more_info"));
      WMenuItem *imagesMenuItem = popup->addItem(WString::tr("objectnames_digitalized_sky_survey_menu"));
      imagesMenuItem->triggered().connect([=](WMenuItem*, _n5) {
        WStackedWidget *stack = AstroPlanner::instance()->widgetsStack();
        WWidget *currentWidget = stack->currentWidget();
        string currentInternalPath = wApp->internalPath();
        WContainerWidget *dssContainer = new DSSPage(object, d->session, [=]{
          stack->setCurrentWidget(currentWidget);
          wApp->setInternalPath(currentInternalPath, true);
        });
        stack->addWidget(dssContainer);
        stack->setCurrentWidget(dssContainer);
      });


      string catName;
      string catNumber;
      for(auto nebula: object->nebulae()) {
	if(nebula->isNgcIc() ) {
	  catName = *nebula->catalogue();
	  catNumber = *nebula->number();
    WMenuItem *ngcIcMenuItem = popup->addItem("NGC-IC Project Page");
    ngcIcMenuItem->setLink(new AutoPostResource{"http://www.ngcicproject.org/ngcicdb.asp", {{"ngcicobject", *object->objectId()}}});
    ngcIcMenuItem->setLinkTarget(TargetNewWindow);
      stringstream dsoBrowserLink;
      dsoBrowserLink << "http://dso-browser.com/dso/info/" << catName << "/" << catNumber;
      dsoBrowserLink << "?year=" << astroSession->when().date().year() 
	<< "&month=" <<astroSession->when().date().month().as_number() 
	<< "&day=" << astroSession->when().date().day();
      if(astroSession->position()) {
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
 

	}
      }
      
     
      popup->addSectionHeader(WString::tr("objectnames_search_menu_title"));
      auto searchURL = [=] (string url, NebulaDenominationPtr nebulaDenomination) {
        return (format(url) % Utils::urlDecode(nebulaDenomination->search())).str();
      };
      if(denominations.size() == 1) {
        addLink(WString::tr("objectnames_google_search"), searchURL("http://www.google.com/search?q=%s", denominations.front() ) );
        addLink(WString::tr("objectnames_google_images_search"), searchURL("http://www.google.com/images?q=%s", denominations.front() ) );
        addLink("SIMBAD", searchURL("http://simbad.u-strasbg.fr/simbad/sim-basic?Ident=%s&submit=SIMBAD+search", denominations.front() ) );
        addLink("NED", searchURL("http://ned.ipac.caltech.edu/cgi-bin/objsearch?objname=%s", denominations.front() ) );
      } else {
        WMenu *googleSearchSubMenu = new WPopupMenu();
        WMenu *googleImagesSearchSubMenu = new WPopupMenu();
        WMenu *simbadSearchSubMenu = new WPopupMenu;
        WMenu *nedSearchSubMenu = new WPopupMenu;
        popup->addMenu(WString::tr("objectnames_google_search"), googleSearchSubMenu);
        popup->addMenu(WString::tr("objectnames_google_images_search"), googleImagesSearchSubMenu);
        popup->addMenu("SIMBAD", simbadSearchSubMenu);
        popup->addMenu("NED", nedSearchSubMenu);
        for(auto name: denominations) {
          addLink(name->search(), searchURL("http://www.google.com/search?q=%s", denominations.front() ), googleSearchSubMenu );
          addLink(name->search(), searchURL("http://www.google.com/images?q=%s", denominations.front() ), googleImagesSearchSubMenu );
          addLink(name->search(), searchURL("http://simbad.u-strasbg.fr/simbad/sim-basic?Ident=%s&submit=SIMBAD+search", denominations.front() ), simbadSearchSubMenu );
          addLink(name->search(), searchURL("http://ned.ipac.caltech.edu/cgi-bin/objsearch?objname=%s", denominations.front() ), nedSearchSubMenu );
        }
      }
      popup->popup(e);
    });
}

ObjectNamesWidget::~ObjectNamesWidget()
{
}
