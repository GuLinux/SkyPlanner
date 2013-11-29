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
#include <Wt/WLineEdit>
#include <boost/format.hpp>
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

MapsWidget::MapsWidget(WLineEdit *searchBox, const JSignal<> &mapReady, WContainerWidget *parent)
  : WGoogleMapMod(Version3, parent)
{
//  wApp->require("https://maps.googleapis.com/maps/api/js?v=3&sensor=false&libraries=places");
  setCenter({45.466667, 9.183333});
  if(searchBox) {
    doGmJavaScript((boost::format(JS(
      var markers = [];
      var map = document.getElementById('%s').map;
      var input = document.getElementById('%s');
      map.controls[google.maps.ControlPosition.TOP_LEFT].push(input);
      var searchBox = new google.maps.places.SearchBox(input);
      
    // copied from google maps api doc: https://developers.google.com/maps/documentation/javascript/examples/places-searchbox
    // [START region_getplaces]
    // Listen for the event fired when the user selects an item from the
    // pick list. Retrieve the matching places for that item.
    google.maps.event.addListener(searchBox, 'places_changed', function() {
      var places = searchBox.getPlaces();

      for (var i = 0, marker; marker = markers[i]; i++) {
	marker.setMap(null);
      }

      // For each place, get the icon, place name, and location.
      markers = [];
      var bounds = new google.maps.LatLngBounds();
      for (var i = 0, place; place = places[i]; i++) {
	var image = {
	  url: place.icon,
	  size: new google.maps.Size(71, 71),
	  origin: new google.maps.Point(0, 0),
	  anchor: new google.maps.Point(17, 34),
	  scaledSize: new google.maps.Size(25, 25)
	};

	// Create a marker for each place.
	var marker = new google.maps.Marker({
	  map: map,
	  icon: image,
	  title: place.name,
	  position: place.geometry.location
	});

	markers.push(marker);

	bounds.extend(place.geometry.location);
      }

      map.fitBounds(bounds);
    });
    // [END region_getplaces]  
    )) % id() % searchBox->id() ).str()
    );
  }
  doGmJavaScript( mapReady.createCall() );
}

void MapsWidget::centerToGeoLocation()
{
  string centerMapToCurrentPlaceJs = (
    boost::format("\n\
    if(navigator.geolocation) { \n\
      navigator.geolocation.getCurrentPosition(function(position) { \n\
      initialLocation = new google.maps.LatLng(position.coords.latitude,position.coords.longitude); \n\
      $('#%s')[0].map.setCenter(initialLocation); \n\
    }); \n\
    }") % id()
  ).str();
  doGmJavaScript(centerMapToCurrentPlaceJs);
}

PlaceWidget::Private::Private(const Wt::Dbo::ptr< AstroSession >& astroSession, Session& session, PlaceWidget* q) 
  : astroSession(astroSession), session(session), mapReady(q, "runOnMapReady"), q(q)
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
  addWidget(new WText("Just click a point on the map to set the observation place. Use the search box to find places by name."));
  WLineEdit *searchBox = WW<WLineEdit>(this).css("controls");
  searchBox->setWidth(500);
  searchBox->setMargin(10);
  MapsWidget *map = new MapsWidget(0, d->mapReady, this);
  map->setHeight(450);
  if(astroSession->position()) {
    d->currentPlace = {astroSession->position().latitude.degrees(), astroSession->position().longitude.degrees()};
    map->setCenter(d->currentPlace);
    map->addMarker(d->currentPlace);
  }
  else {
    map->centerToGeoLocation();
  }
  map->clicked().connect([=](WGoogleMapMod::Coordinate c, _n5){
    map->clearOverlays();
    map->addMarker(c);
    Dbo::Transaction t(d->session);
    astroSession.modify()->setPosition(Coordinates::LatLng{Angle::degrees(c.latitude()), Angle::degrees(c.longitude())});
    t.commit();
    d->placeChanged.emit(c.latitude(), c.longitude());
  });
}

JSignal<> &PlaceWidget::mapReady() const
{
  return d->mapReady;
}

Signal< double, double >& PlaceWidget::placeChanged() const
{
  return d->placeChanged;
}

