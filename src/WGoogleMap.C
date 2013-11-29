// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 *
 * Contributed by: Richard Ulrich.
 */

#include "WGoogleMap"
#include <Wt/WApplication>
#include <Wt/WContainerWidget>

#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

#include <string>
#include <utility>
#include <iostream>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace {
  // if there is no google api key configured, use the one for
  // http://localhost:8080/
  static const std::string localhost_key
    = "ABQIAAAAWqrN5o4-ISwj0Up_depYvhTwM0brOpm-"
      "All5BF6PoaKBxRWWERS-S9gPtCri-B6BZeXV8KpT4F80DQ";
}

namespace Wt {


// example javascript code from :
// http://code.google.com/apis/maps/documentation/

WGoogleMapMod::WGoogleMapMod(WContainerWidget *parent)
 : clicked_(this, "click"),
   doubleClicked_(this, "dblclick"),
   mouseMoved_(0)
{
  setImplementation(new WContainerWidget());

  if (parent)
    parent->addWidget(this);
}

WGoogleMapMod::~WGoogleMapMod()
{ 
  delete mouseMoved_;
}

void WGoogleMapMod::streamJSListener(const JSignal<Coordinate> &signal, 
				  std::string signalName,
				  Wt::WStringStream &strm) 
{
  strm << 
    """google.maps.event.addListener(map, \"" << signalName << "\", "
    ""                              "function(event) {"
    ""  "if (event && event.latLng) {"
        << signal.createCall("event.latLng.lat() +' '+ event.latLng.lng()") 
        << ";"
    ""  "}"
    """});";
}

JSignal<WGoogleMapMod::Coordinate>& WGoogleMapMod::mouseMoved()
{
  if (!mouseMoved_)
    mouseMoved_ = new JSignal<Coordinate>(this, "mousemove");

  return *mouseMoved_;
}

void WGoogleMapMod::render(WFlags<RenderFlag> flags)
{
  if (flags & RenderFull) {
    WApplication *app = WApplication::instance();

    std::string googlekey = localhost_key;
    Wt::WApplication::readConfigurationProperty("google_api_key", googlekey);
      
    // init the google javascript api
    const std::string gmuri = "http://www.google.com/jsapi?key=" + googlekey;
    app->require(gmuri, "google");

    std::string initFunction = 
      app->javaScriptClass() + ".init_google_gum_maps_" + id();

    // initialize the map
    WStringStream strm;
    strm <<
      "{ " << initFunction << " = function() {"
      """var self = " << jsRef() << ";"
      """if (!self) { "
      ""   "setTimeout(" << initFunction << ", 0);"
      ""   "return;"
      "}";

    strm << 
      "var latlng = new google.maps.LatLng(47.01887777, 8.651888);"
      "var myOptions = {"
      "" "zoom: 13,"
      "" "center: latlng,"
      "" "mapTypeId: google.maps.MapTypeId.ROADMAP"
      "};"
      "var map = new google.maps.Map(self, myOptions);"
      "map.overlays = [];"
      "map.infowindows = [];";
    setJavaScriptMember(WT_RESIZE_JS,
                        "function(self, w, h) {"
                        """if (w >= 0) "
                        ""  "self.style.width=w + 'px';"
                        """if (h >= 0) "
                        ""  "self.style.height=h + 'px';"
                        """if (self.map)"
                        """ google.maps.event.trigger(self.map, 'resize');"
                        "}");
    strm << "self.map = map;";

    // eventhandling
    streamJSListener(clicked_, "click", strm);
    streamJSListener(doubleClicked_, "dblclick", strm);
    if (mouseMoved_)
      streamJSListener(*mouseMoved_, "mousemove", strm);

    // additional things
    for (unsigned int i = 0; i < additions_.size(); i++)
      strm << additions_[i];

    strm << "setTimeout(function(){ delete " << initFunction << ";}, 0)};"
	 << "google.load(\"maps\", \"3.exp\", {other_params:\"sensor=false&libraries=places\", callback: "
	 << initFunction << "});"
	 << "}"; // private scope

    additions_.clear();

    app->doJavaScript(strm.str(), true);
  }

  WCompositeWidget::render(flags);
}

void WGoogleMapMod::clearOverlays()
{
  std::stringstream strm;
  strm 
    << "var mapLocal = " << jsRef() + ".map, i;\n"
    << "if (mapLocal.overlays) {\n"
    << """for (i in mapLocal.overlays) {\n"
    << """mapLocal.overlays[i].setMap(null);\n"
    << "}\n"
    << "mapLocal.overlays.length = 0;\n"
    << "}\n"
    << "if (mapLocal.infowindows) {\n"
    << """for (i in mapLocal.infowindows) {\n"
    << ""  "mapLocal.infowindows[i].close();\n"
    << ""  "}\n"
    << """mapLocal.infowindows.length = 0;\n"
    << "}\n";

  doGmJavaScript(strm.str());
}

void WGoogleMapMod::doGmJavaScript(const std::string& jscode)
{
  if (isRendered())
    doJavaScript(jscode);
  else
    additions_.push_back(jscode);
}

void WGoogleMapMod::addMarker(const Coordinate& pos)
{
  std::stringstream strm;
  strm << "var position = new google.maps.LatLng("
        << pos.latitude() << ", " << pos.longitude() << ");"
        << "var marker = new google.maps.Marker({"
        << "position: position,"
        << "map: " << jsRef() << ".map"
        << "});"
        << jsRef() << ".map.overlays.push(marker);";

  doGmJavaScript(strm.str());
}

void WGoogleMapMod::addIconMarker(const Coordinate &pos,
                               const std::string& iconURL)
{
  std::stringstream strm;
  strm << "var position = new google.maps.LatLng("
        << pos.latitude() << ", " << pos.longitude() << ");"
    
        << "var marker = new google.maps.Marker({"
        << "position: position,"
        << "icon: \"" <<  iconURL << "\","
        << "map: " << jsRef() << ".map"
        << "});"
    
        << jsRef() << ".map.overlays.push(marker);";
 
  doGmJavaScript(strm.str());
}

void WGoogleMapMod::addCircle(const Coordinate& center, double radius, 
			   const WColor& strokeColor, int strokeWidth,
			   const WColor& fillColor)
{
  std::stringstream strm;

  double strokeOpacity = strokeColor.alpha() / 255.0;
  double fillOpacity = fillColor.alpha() / 255.0;
  
  strm << "var mapLocal = " << jsRef() + ".map;"
        << "var latLng  = new google.maps.LatLng(" 
        << center.latitude() << "," << center.longitude() << ");"
        << "var circle = new google.maps.Circle( "
          "{ "
          "  map: mapLocal, "
          "  radius: " << radius << ", "
          "  center:  latLng  ,"
          "  fillOpacity: \"" << fillOpacity << "\","
          "  fillColor: \"" << fillColor.cssText() << "\","
          "  strokeWeight: " << strokeWidth << ","
          "  strokeColor:\"" << strokeColor.cssText() << "\","
          "  strokeOpacity: " << strokeOpacity <<
          "} "
          ");";

  doGmJavaScript(strm.str());
}

void WGoogleMapMod::addPolyline(const std::vector<Coordinate>& points,
			     const WColor& color, int width, double opacity)
{
  if (opacity == 1.0)
    opacity = color.alpha() / 255.0;

  // opacity has to be between 0.0 and 1.0
  opacity = std::max(std::min(opacity, 1.0), 0.0);

  std::stringstream strm;
  strm << "var waypoints = [];";
  for (size_t i = 0; i < points.size(); ++i)
    strm << "waypoints[" << i << "] = new google.maps.LatLng("
	 << points[i].latitude() << ", " << points[i].longitude() << ");";

  strm << 
    "var poly = new google.maps.Polyline({"
    "path: waypoints,"
    "strokeColor: \"" << color.cssText() << "\"," <<
    "strokeOpacity: " << opacity << "," << 
    "strokeWeight: " << width <<
    "});" <<
    "poly.setMap(" << jsRef() << ".map);" <<
    jsRef() << ".map.overlays.push(poly);";

  doGmJavaScript(strm.str());
}

void WGoogleMapMod::openInfoWindow(const Coordinate& pos,
				const WString& myHtml)
{
  std::stringstream strm;
  strm << "var pos = new google.maps.LatLng(" 
       << pos.latitude() << ", " << pos.longitude() << ");";
  
  strm << "var infowindow = new google.maps.InfoWindow({"
    "content: " << WWebWidget::jsStringLiteral(myHtml) << "," <<
    "position: pos"
    "});"
    "infowindow.open(" << jsRef() << ".map);" <<
    jsRef() << ".map.infowindows.push(infowindow);";

  doGmJavaScript(strm.str());
}

void WGoogleMapMod::setCenter(const Coordinate& center)
{
  std::stringstream strm;
  strm << jsRef() << ".map.setCenter(new google.maps.LatLng("
       << center.latitude() << ", " << center.longitude() << "));";

  doGmJavaScript(strm.str());
}

void WGoogleMapMod::setCenter(const Coordinate& center, int zoom)
{
  std::stringstream strm;
  strm << jsRef() << ".map.setCenter(new google.maps.LatLng("
       << center.latitude() << ", " << center.longitude() << ")); "
       << jsRef() << ".map.setZoom(" << zoom << ");";

  doGmJavaScript(strm.str());
}

void WGoogleMapMod::panTo(const Coordinate& center)
{
  std::stringstream strm;
  strm << jsRef() << ".map.panTo(new google.maps.LatLng("
       << center.latitude() << ", " << center.longitude() << "));";

  doGmJavaScript(strm.str());
}

void WGoogleMapMod::setZoom(int level)
{
  doGmJavaScript(jsRef() + ".map.setZoom("
		 + boost::lexical_cast<std::string>(level) + ");");
}

void WGoogleMapMod::zoomIn()
{
  std::stringstream strm;
  strm 
    << "var zoom = " << jsRef() << ".map.getZoom();"
    << jsRef() << ".map.setZoom(zoom + 1);";
  doGmJavaScript(strm.str());
}

void WGoogleMapMod::zoomOut()
{
  std::stringstream strm;
  strm 
    << "var zoom = " << jsRef() << ".map.getZoom();"
    << jsRef() << ".map.setZoom(zoom - 1);";
  doGmJavaScript(strm.str());
}

void WGoogleMapMod::savePosition()
{
  std::stringstream strm;
  strm
    << jsRef() << ".map.savedZoom = " << jsRef() << ".map.getZoom();"
    << jsRef() << ".map.savedPosition = " << jsRef() << ".map.getCenter();";
  doGmJavaScript(strm.str());
}

void WGoogleMapMod::returnToSavedPosition()
{
  std::stringstream strm;
  strm
    << jsRef() << ".map.setZoom(" << jsRef() << ".map.savedZoom);"
    << jsRef() << ".map.setCenter(" << jsRef() << ".map.savedPosition);";
  doGmJavaScript(strm.str());
}

void WGoogleMapMod::checkResize()
{
  doGmJavaScript(jsRef() + ".map.checkResize();");
}

void WGoogleMapMod::setMapOption(const std::string &option, 
			      const std::string &value)
{
  std::stringstream strm;
  strm
    << "var option = {"
    << option << " :" << value
    << "};"
    << jsRef() << ".map.setOptions(option);";

  doGmJavaScript(strm.str());
}

void WGoogleMapMod::enableDragging()
{
  setMapOption("draggable", "true");
}

void WGoogleMapMod::disableDragging()
{
  setMapOption("draggable", "false");
}

void WGoogleMapMod::enableDoubleClickZoom()
{
  setMapOption("disableDoubleClickZoom", "false");
}

void WGoogleMapMod::disableDoubleClickZoom()
{
  setMapOption("disableDoubleClickZoom", "true");
}

void WGoogleMapMod::enableScrollWheelZoom()
{
  setMapOption("scrollwheel", "true");
}

void WGoogleMapMod::disableScrollWheelZoom()
{
  setMapOption("scrollwheel", "false");
}

#ifndef WT_TARGET_JAVA
void WGoogleMapMod::zoomWindow(const std::pair<Coordinate, Coordinate>& bbox)
{
  zoomWindow(bbox.first, bbox.second);
}
#endif

void WGoogleMapMod::zoomWindow(const Coordinate& topLeft, 
			    const Coordinate& rightBottom)
{
  const Coordinate center
    ((topLeft.latitude() + rightBottom.latitude()) / 2.0,
     (topLeft.longitude() + rightBottom.longitude()) / 2.0);

  Coordinate topLeftC = 
    Coordinate(std::min(topLeft.latitude(), rightBottom.latitude()),
	       std::min(topLeft.longitude(), rightBottom.longitude()));
  Coordinate rightBottomC = 
    Coordinate(std::max(topLeft.latitude(), rightBottom.latitude()),
	       std::max(topLeft.longitude(), rightBottom.longitude()));
  std::stringstream strm;
  strm << "var bbox = new google.maps.LatLngBounds(new google.maps.LatLng("
       << topLeftC.latitude()  << ", " << topLeftC.longitude() << "), "
       << "new google.maps.LatLng("
       << rightBottomC.latitude() << ", " << rightBottomC.longitude() << "));";

  strm 
    << jsRef() << ".map.fitBounds(bbox);";

  doGmJavaScript(strm.str());
}

void WGoogleMapMod::setMapTypeControl(MapTypeControl type)
{
  std::stringstream strm;
  std::string control;
  switch (type) {
  case DefaultControl:
    control = "DEFAULT";
    break;
  case MenuControl:
    control = "DROPDOWN_MENU";
    break;
  case HorizontalBarControl:
    control = "HORIZONTAL_BAR";
    break;
  default:
    control = "";
  }

  strm 
    << "var options = {"
    << """disableDefaultUI: " << (control == "" ? "true" : "false") << ","
    << ""  "mapTypeControlOptions: {";

  if (control != "")
    strm << "style: google.maps.MapTypeControlStyle." << control;

  strm 
    << """}"
    << "};"
    << jsRef() << ".map.setOptions(options);";
  
  doGmJavaScript(strm.str());
}

}

