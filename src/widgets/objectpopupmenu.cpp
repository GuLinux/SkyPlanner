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

#include "widgets/private/objectpopupmenu_p.h"
#include "utils/d_ptr_implementation.h"

#include <Wt/WLink>
#include <Wt/WApplication>
#include <Wt/WDialog>
#include <Wt/WText>
#include <Wt/Utils>
#include <Wt/WPushButton>

#include "Wt-Commons/wt_helpers.h"
#include <boost/algorithm/string/join.hpp>
#include "widgets/astroobjectwidget.h"
#include "types.h"
#include "widgets/dsspage.h"
#include "utils/format.h"
#include "utils/autopostresource.h"
#include "skyplanner.h"
#include "sendfeedbackpage.hpp"

using namespace std;
using namespace Wt;
using namespace WtCommons;

ObjectPopupMenu::Private::Private(ObjectPopupMenu *q) : q(q)
{
}
 
ObjectPopupMenu::~ObjectPopupMenu()
{
}


ObjectPopupMenu::ObjectPopupMenu(const NgcObjectPtr &object,  const AstroSessionPtr &astroSession, const TelescopePtr &telescope, const Timezone &timezone, Session &session)
  : WPopupMenu(nullptr), d(this)
{
    Dbo::Transaction t( session );
    auto addLink = [=]( const WString & label, const WLink & url, WMenu *menu = 0 )
    {
      if( !menu )
        menu = this;

      WMenuItem *menuItem = menu->addItem( label );
      menuItem->setLink( url );
      if(url.type() != WLink::InternalPath)
        menuItem->setLinkTarget( TargetNewWindow );
    };
    addSectionHeader( WString::tr( "objectnames_more_info" ) )->addStyleClass("dropdown-header");
    string haveGis;
    if( wApp->readConfigurationProperty("have-gis", haveGis) && haveGis == "true" ) {
      addItem(WString::tr("objectnames_nearby_objects"))->triggered().connect([=,&session](WMenuItem*, _n5){
	Dbo::Transaction t( session );
	WDialog *dialog = new WDialog{ WString::tr("objectnames_nearby_objects_caption").arg( WString::fromUTF8(NgcObject::namesByCatalogueImportance(t, object)[0]) ) };
	dialog->setClosable(true); 
//	dialog->setResizable(true); 
	WMenu *namesList = WW<WMenu>().css("col-xs-3");
	WContainerWidget *preview = WW<WContainerWidget>().css("col-xs-9");
	dialog->contents()->addWidget(WW<WContainerWidget>().css("container").add(namesList).add(preview));
	string query = R"( select o, 
	  ST_Distance(coordinates_geom, (select coordinates_geom FROM objects where id = ? ) ) dist
	  from objects o where o.id <> ? order by dist asc )";
	auto objectsDbo = session.query<boost::tuple<NgcObjectPtr, double>>(query).bind(object.id()).bind(object.id()).limit(15).resultList();
	for(auto o: objectsDbo) {
	  auto names = NgcObject::namesByCatalogueImportance(t, o.get<0>());
	  names.resize(min(names.size(), size_t{3}));
	  auto item = namesList->addItem(boost::algorithm::join(names, ", "));
	  item->addStyleClass("nearby-object-names");
	  item->triggered().connect([=,&session](WMenuItem*, _n5){
	    preview->clear();
	    vector<WPushButton*> actions{ WW<WPushButton>(WString::tr("buttons_close")).css("btn-sm").onClick([=](WMouseEvent){ preview->clear(); }).get() };
	    
	    Dbo::Transaction t( session );
	    if(astroSession && astroSession->astroSessionObjects().find().where("objects_id = ?").bind(o.get<0>().id()).resultList().size() == 0 ) {
	      auto addToSessionAction = [=,&session](WMouseEvent){
		Dbo::Transaction t( session );
		wApp->log("notice") << "Adding object to session...";
	      };
	      actions.insert(actions.begin(), WW<WPushButton>(WString::tr("buttons_add")).css("btn-sm btn-success").onClick(addToSessionAction));
	    }
	    preview->addWidget(new AstroObjectWidget{o.get<0>(), astroSession, session, timezone, telescope, {}, actions});
	  });
	}
	dialog->show();
      });
    }
    WMenuItem *imagesMenuItem = addItem( WString::tr( "objectnames_digitalized_sky_survey_menu" ) );
    imagesMenuItem->setLink(WLink(WLink::InternalPath, DSSPage::internalPath(object, t)));

    string catName;
    string catNumber;
    auto dboDenominations = NgcObject::denominationsByCatalogueImportance(t, object);
    vector<NebulaDenominationPtr> denominations{begin(dboDenominations), end(dboDenominations)};
    for( auto nebula : denominations )
    {
      if( nebula->isNgcIc() )
      {
        catName = nebula->catalogue()->name();
        catNumber = *nebula->number();
        WMenuItem *ngcIcMenuItem = addItem( "NGC-IC Project Page" );
        ngcIcMenuItem->setLink( new AutoPostResource {"http://www.ngcicproject.org/ngcicdb.asp", {{"ngcicobject", format("%s%s") % nebula->catalogue()->code() % catNumber}}} );
        ngcIcMenuItem->setLinkTarget( TargetNewWindow );
        stringstream dsoBrowserLink;
        dsoBrowserLink << "http://dso-browser.com/dso/info/" << catName << "/" << catNumber;
        if(astroSession) {
          dsoBrowserLink << "?year=" << astroSession->when().date().year()
          << "&month=" << astroSession->when().date().month().as_number()
          << "&day=" << astroSession->when().date().day();
        }

        if( astroSession && astroSession->position() )
        {
          string longitudeEmisphere = astroSession->position().longitude.degrees() > 0 ? "E" : "W" ;
          Angle::Sexagesimal longitude = astroSession->position().longitude.degrees() > 0
                                         ? astroSession->position().longitude.sexagesimal()
                                         : Angle::degrees( -astroSession->position().longitude.degrees() ).sexagesimal();

          string latitudeEmisphere = astroSession->position().latitude.degrees() > 0 ? "N" : "S";
          Angle::Sexagesimal latitude = ( astroSession->position().latitude.degrees() > 0 )
                                        ? astroSession->position().latitude.sexagesimal()
                                        : Angle::degrees( -astroSession->position().latitude.degrees() ).sexagesimal();
          dsoBrowserLink << "&lat_deg=" << latitude.degrees << "&lat_min=" << latitude.minutes << "&lat_sec=" << static_cast<int>( latitude.seconds ) << "&lon_hem=" << longitudeEmisphere;
          dsoBrowserLink << "&lon_deg=" << longitude.degrees << "&lon_min=" << longitude.minutes << "&lon_sec=" << static_cast<int>( longitude.seconds ) << "&lat_hem=" << latitudeEmisphere;
        }

        addLink( "DSO Browser", dsoBrowserLink.str() );
      }
    }


    addSectionHeader( WString::tr( "objectnames_search_menu_title" ) )->addStyleClass("dropdown-header");
    auto searchURL = [ = ]( string url, NebulaDenominationPtr nebulaDenomination )
    {
      return ( format( url ) % Utils::urlDecode( nebulaDenomination->search() ) ).str();
    };

    if( denominations.size() == 1 )
    {
      addLink( WString::tr( "objectnames_google_search" ), searchURL( "http://www.google.com/search?q=%s", denominations.front() ) );
      addLink( WString::tr( "objectnames_google_images_search" ), searchURL( "http://www.google.com/images?q=%s", denominations.front() ) );
      addLink( "SIMBAD", searchURL( "http://simbad.u-strasbg.fr/simbad/sim-basic?Ident=%s&submit=SIMBAD+search", denominations.front() ) );
      addLink( "NED", searchURL( "http://ned.ipac.caltech.edu/cgi-bin/objsearch?objname=%s", denominations.front() ) );
    }
    else
    {
      WMenu *googleSearchSubMenu = new WPopupMenu();
      WMenu *googleImagesSearchSubMenu = new WPopupMenu();
      WMenu *simbadSearchSubMenu = new WPopupMenu;
      WMenu *nedSearchSubMenu = new WPopupMenu;
      addMenu( WString::tr( "objectnames_google_search" ), googleSearchSubMenu );
      addMenu( WString::tr( "objectnames_google_images_search" ), googleImagesSearchSubMenu );
      addMenu( "SIMBAD", simbadSearchSubMenu );
      addMenu( "NED", nedSearchSubMenu );

      for( auto name : denominations )
      {
        addLink( WString::fromUTF8(name->search()), searchURL( "http://www.google.com/search?q=%s", name), googleSearchSubMenu );
        addLink( WString::fromUTF8(name->search()), searchURL( "http://www.google.com/images?q=%s", name ), googleImagesSearchSubMenu );
        addLink( WString::fromUTF8(name->search()), searchURL( "http://simbad.u-strasbg.fr/simbad/sim-basic?Ident=%s&submit=SIMBAD+search", name ), simbadSearchSubMenu );
        addLink( WString::fromUTF8(name->search()), searchURL( "http://ned.ipac.caltech.edu/cgi-bin/objsearch?objname=%s", name ), nedSearchSubMenu );
      }
    }

    if(session.user()) {
      addSectionHeader( WString::tr( "objectnames_feedback_title" ) )->addStyleClass("dropdown-header");
      addLink(WString::tr( "objectnames_feedback_menu" ), WLink(WLink::InternalPath, SendFeedbackPage::internalPath(object, &t)) );
    }
}