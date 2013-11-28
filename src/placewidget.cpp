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

#include "Models"
#include "placewidget.h"
#include "private/placewidget_p.h"
#include "utils/d_ptr_implementation.h"
#include "session.h"
#include "Wt-Commons/wt_helpers.h"
#include <Wt/WApplication>
#include <Wt/WPushButton>
using namespace std;
using namespace Wt;
using namespace WtCommons;
/*
  map->doJavaScript((boost::format(
     "var searchBox = new google.maps.places.SearchBox(document.getElementById('%d')); \
     google.maps.event.addListener(searchBox, 'places_changed', function() { \
     var places = searchBox.getPlaces();\
     for (var i = 0, place; place = places[i]; i++) {\
       console.log(\"Place: \" + place.name + \", location: \" + place.geometry.location);\
      }\
     });\
     ")
    % searchPlace->id()
  ).str());
  */

MapsWidget::MapsWidget(WContainerWidget *parent)
  : WGoogleMap(Version3, parent)
{
//  wApp->require("https://maps.googleapis.com/maps/api/js?v=3&sensor=false&libraries=places");
}

PlaceWidget::Private::Private(const Wt::Dbo::ptr< AstroSession >& astroSession, Session& session, PlaceWidget* q) 
  : astroSession(astroSession), session(session), q(q)
{
}

PlaceWidget::~PlaceWidget()
{
}

PlaceWidget::PlaceWidget(const Wt::Dbo::ptr< AstroSession >& astroSession, Session& session, Wt::WContainerWidget* parent)
    : d(astroSession, session, this)
{
//   WPushButton *setPlaceButton = WW<WPushButton>("Set Current Position").css("btn btn-primary").onClick([=](WMouseEvent){
//     Dbo::Transaction t(d->session);
//     astroSession.modify()->setPosition({});
//   }).disable();
//   addWidget(setPlaceButton);
  addWidget(new WText("For the time being, you have to manually find the observation place, and click a point to set it."));
  MapsWidget *map = new MapsWidget(this);
  map->setHeight(400);
  if(astroSession->position()) {
    d->currentPlace = {astroSession->position().latitude.degrees(), astroSession->position().longitude.degrees()};
    map->setCenter(d->currentPlace);
    map->addMarker(d->currentPlace);
  }
  map->clicked().connect([=](WGoogleMap::Coordinate c, _n5){
    map->clearOverlays();
    map->addMarker(c);
    Dbo::Transaction t(d->session);
    astroSession.modify()->setPosition(Coordinates::LatLng{Angle::degrees(c.latitude()), Angle::degrees(c.longitude())});
    t.commit();
    d->placeChanged.emit(c.latitude(), c.longitude());
  });
}

Signal< double, double >& PlaceWidget::placeChanged() const
{
  return d->placeChanged;
}

