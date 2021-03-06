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
#include "Models"
#include "placewidget.h"
#include "private/placewidget_p.h"
#include "utils/format.h"
#include "session.h"
#include "wt_helpers.h"
#include <Wt/WApplication>
#include <Wt/WPushButton>
#include <Wt/WLineEdit>
#include <boost/format.hpp>
using namespace std;
using namespace Wt;
using namespace WtCommons;

MapsWidget::MapsWidget(WLineEdit *searchBox, const JSignal<> &mapReady, WContainerWidget *parent)
  : WGoogleMapMod(parent)
{
  setCenter({45.466667, 9.183333});
  if(searchBox) {
    doGmJavaScript(format(JS(
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
    )) % id() % searchBox->id() );
  }
  doGmJavaScript( mapReady.createCall() );
}

void MapsWidget::centerToGeoLocation()
{
  string centerMapToCurrentPlaceJs = format("\n\
    if(navigator.geolocation) { \n\
      navigator.geolocation.getCurrentPosition(function(position) { \n\
      initialLocation = new google.maps.LatLng(position.coords.latitude,position.coords.longitude); \n\
      $('#%s')[0].map.setCenter(initialLocation); \n\
    }); \n\
    }") % id();
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
    : dptr(astroSession, session, this)
{
//  addWidget(new WText(WString::tr("placewidget_maps_instructions")));
  WContainerWidget *googleMapsLinkContainer = new WContainerWidget;
  addWidget(googleMapsLinkContainer);
  auto linkToGoogleMaps = [=](double latitude, double longitude) {
    googleMapsLinkContainer->clear();
    string url = format("http://maps.google.com/maps?q=%f,%f1") % latitude % longitude;
    auto link = new WAnchor(url, WString::tr("placewidget_open_google_maps"));
    link->setTarget(TargetNewWindow);
    googleMapsLinkContainer->addWidget(link);
  };
  WLineEdit *searchBox = WW<WLineEdit>(this).css("controls");
  searchBox->setWidth(500);
  searchBox->setMargin(10);
  MapsWidget *map = new MapsWidget(searchBox, d->mapReady);
  map->setHeight(450);
  addWidget(map);
  if(astroSession->position()) {
    WGoogleMap::Coordinate currentPlace = {astroSession->position().latitude.degrees(), astroSession->position().longitude.degrees()};
    map->setCenter(currentPlace);
    map->addMarker(currentPlace);
    linkToGoogleMaps(currentPlace.latitude(), currentPlace.longitude());
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
    linkToGoogleMaps(c.latitude(), c.longitude());
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

