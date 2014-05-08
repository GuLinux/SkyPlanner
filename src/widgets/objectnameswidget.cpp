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
#include "skyplanner.h"
#include <boost/regex.hpp>
#include <boost/algorithm/string/join.hpp>
#include "widgets/dsspage.h"
#include "sendfeedbackpage.hpp"
#include "widgets/astroobjectwidget.h"

using namespace WtCommons;
using namespace Wt;
using namespace std;

class ObjectNamesWidget::Private
{
  public:
    Private( Session &session, ObjectNamesWidget *q ) : session( session ), q( q ) {}
    AstroSessionObjectPtr astroSessionObject;
    Session &session;
    void init(const NgcObjectPtr &object, const AstroSessionPtr &astroSession, RenderType renderType, int limitNames);
  private:
    ObjectNamesWidget *const q;
};


ObjectNamesWidget::ObjectNamesWidget( const NgcObjectPtr &object, Session &session, const AstroSessionPtr &astroSession, RenderType renderType, int limitNames, WContainerWidget *parent )
  : WContainerWidget( parent ), d( session, this )
{
  d->init(object, astroSession, renderType, limitNames);
}

void ObjectNamesWidget::Private::init(const NgcObjectPtr &object, const AstroSessionPtr &astroSession, RenderType renderType, int limitNames)
{
  Dbo::Transaction t(session);
  auto names = NgcObject::namesByCatalogueImportance(t, object);
  if(limitNames > 0)
    names.resize(limitNames);
  WString namesJoined = Utils::htmlEncode( WString::fromUTF8( boost::algorithm::join( names, ", " ) ) );

  if( renderType == Printable )
  {
    q->setInline( true );
    q->addWidget( new WText {namesJoined} );
    return;
  }

  WAnchor *namesText = WW<WAnchor>( "", namesJoined ).css( "link" );
  q->addWidget( namesText );

  namesText->clicked().connect( [ = ]( WMouseEvent e )
  {
    Dbo::Transaction t( session );
    WPopupMenu *popup = new WPopupMenu();
    auto addLink = [ = ]( const WString & label, const WLink & url, WMenu *menu = 0 )
    {
      if( !menu )
        menu = popup;

      WMenuItem *menuItem = menu->addItem( label );
      menuItem->setLink( url );
      if(url.type() != WLink::InternalPath)
        menuItem->setLinkTarget( TargetNewWindow );
    };
    popup->addSectionHeader( WString::tr( "objectnames_more_info" ) )->addStyleClass("dropdown-header");
    WMenuItem *imagesMenuItem = popup->addItem( WString::tr( "objectnames_digitalized_sky_survey_menu" ) );
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
        WMenuItem *ngcIcMenuItem = popup->addItem( "NGC-IC Project Page" );
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


    popup->addSectionHeader( WString::tr( "objectnames_search_menu_title" ) )->addStyleClass("dropdown-header");
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
      popup->addMenu( WString::tr( "objectnames_google_search" ), googleSearchSubMenu );
      popup->addMenu( WString::tr( "objectnames_google_images_search" ), googleImagesSearchSubMenu );
      popup->addMenu( "SIMBAD", simbadSearchSubMenu );
      popup->addMenu( "NED", nedSearchSubMenu );

      for( auto name : denominations )
      {
        addLink( WString::fromUTF8(name->search()), searchURL( "http://www.google.com/search?q=%s", name), googleSearchSubMenu );
        addLink( WString::fromUTF8(name->search()), searchURL( "http://www.google.com/images?q=%s", name ), googleImagesSearchSubMenu );
        addLink( WString::fromUTF8(name->search()), searchURL( "http://simbad.u-strasbg.fr/simbad/sim-basic?Ident=%s&submit=SIMBAD+search", name ), simbadSearchSubMenu );
        addLink( WString::fromUTF8(name->search()), searchURL( "http://ned.ipac.caltech.edu/cgi-bin/objsearch?objname=%s", name ), nedSearchSubMenu );
      }
    }

    if(session.user()) {
      popup->addSectionHeader( WString::tr( "objectnames_feedback_title" ) )->addStyleClass("dropdown-header");
      addLink(WString::tr( "objectnames_feedback_menu" ), WLink(WLink::InternalPath, SendFeedbackPage::internalPath(object, &t)) );
    }
    popup->popup(e);
  } );
}


ObjectNamesWidget::~ObjectNamesWidget()
{
}
