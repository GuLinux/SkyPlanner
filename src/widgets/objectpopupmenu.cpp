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

#include "widgets/private/objectpopupmenu_p.h"
#include "utils/d_ptr_implementation.h"

#include <Wt/WLink>
#include <Wt/WApplication>
#include <Wt/WDialog>
#include <Wt/WText>
#include <Wt/Utils>
#include <Wt/WPushButton>

#include "wt_helpers.h"
#include <boost/algorithm/string/join.hpp>
#include "widgets/astroobjectwidget.h"
#include "types.h"
#include "widgets/dsspage.h"
#include "utils/format.h"
#include "utils/autopostresource.h"
#include "skyplanner.h"
#include "sendfeedbackpage.hpp"
#include "astrosessiontab.h"

using namespace std;
using namespace Wt;
using namespace WtCommons;

ObjectPopupMenu::Private::Private(ObjectPopupMenu *q) : q(q)
{
}
 
ObjectPopupMenu::~ObjectPopupMenu()
{
}


ObjectPopupMenu::ObjectPopupMenu(const AstroGroup &astroGroup, Session &session)
  : WPopupMenu(nullptr), d(this)
{
  auto astroSession = astroGroup.astroSession();
  auto object = astroGroup.object();
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
	dialog->setWidth(1200);
	dialog->setResizable(true); 
	WMenu *namesList = WW<WMenu>();
	WContainerWidget *namesListContainer = WW<WContainerWidget>().css("col-xs-3").add(namesList);
	WContainerWidget *preview = WW<WContainerWidget>().css("col-xs-9");
	dialog->contents()->addStyleClass("nearby-object-modal-body");
	dialog->contents()->addWidget(WW<WContainerWidget>().css("container-fluid nearby-object-container").add(namesListContainer).add(preview));
	auto populateNearbyObjects = [=,&session] (int pageSize, int pageNumber) {
	  Dbo::Transaction t( session );
	  string query = R"( select o from objects o where o.id <> ?
	  order by ST_Distance(coordinates_geom, (select coordinates_geom FROM objects where id = ? ) ) asc )";
	  for(auto item: namesList->items()) delete item;
          preview->clear();
	  auto objectsDbo = session.query<NgcObjectPtr>(query).bind(object.id()).bind(object.id()).limit(pageSize).offset(pageSize*pageNumber).resultList();
	  for(auto objectDbo: objectsDbo) {
	    auto names = NgcObject::namesByCatalogueImportance(t, objectDbo);
	    names.resize(min(names.size(), size_t{3}));
	    auto item = namesList->addItem(WString::fromUTF8(boost::algorithm::join(names, ", ")));
	    item->addStyleClass("nearby-object-names");
	    item->clicked().connect([=,&session](WMouseEvent){
	      preview->clear();
	      vector<WPushButton*> actions{ WW<WPushButton>(WString::tr("buttons_close")).css("btn-xs").onClick([=](WMouseEvent){ preview->clear(); }).get() };
	      
	      Dbo::Transaction t( session );
	      if(astroSession && astroSession->astroSessionObjects().find().where("objects_id = ?").bind(objectDbo.id()).resultList().size() == 0 ) {
		WPushButton *addToSessionButton = WW<WPushButton>(WString::tr("buttons_add")).css("btn-xs btn-success");
		addToSessionButton->clicked().connect([=,&session](WMouseEvent){
		  auto astroSessionObject = AstroSessionTab::add(objectDbo, astroSession, session, item);
		  if(astroSessionObject) {
		    d->objectsListChanged.emit(astroSessionObject);
		    delete addToSessionButton;
		  }
		});
		actions.insert(actions.begin(), addToSessionButton);
	      }
	      preview->addWidget(new AstroObjectWidget{{astroGroup.astroSession(), objectDbo, astroGroup.telescope, astroGroup.timezone}, session, {}, actions});
	    });
	  }
	};
	auto pagination = WW<WMenu>(Horizontal).css("pagination pagination-sm").get();
	int pagesize = 15;
	for(int i=0; i<7; i++) {
	  auto item = pagination->addItem(format("%d") % (i+1) );
	  if(i == 0) pagination->select(item);
	  item->triggered().connect([=] (WMenuItem*, _n5) {
	    populateNearbyObjects(pagesize, i);
	  });
	}
	namesListContainer->addWidget(pagination);
	populateNearbyObjects(pagesize, 0);
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


Signal<AstroSessionObjectPtr> &ObjectPopupMenu::objectsListChanged() const
{
  return d->objectsListChanged;
}