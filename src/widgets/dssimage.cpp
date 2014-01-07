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

#include "dssimage.h"
#include "private/dssimage_p.h"
#include "utils/d_ptr_implementation.h"
#include <Wt/WContainerWidget>
#include <Wt/WAnchor>
#include <Wt/WApplication>
#include <Wt/WImage>
#include <Wt/WText>
#include <Wt/Http/Client>
#include "utils/format.h"
#include <utils/utils.h>
#include <boost/filesystem.hpp>
#include "astroplanner.h"
#include <Wt-Commons/wt_helpers.h>

using namespace Wt;
using namespace WtCommons;
using namespace std;
namespace fs = boost::filesystem;


std::map<DSSImage::ImageVersion,std::string> DSSImage::Private::imageVersionStrings{
  { poss2ukstu_red, "poss2ukstu_red" },
  { poss2ukstu_blue, "poss2ukstu_blue" },
  { poss2ukstu_ir, "poss2ukstu_ir" },
  { poss1_red, "poss1_red" },
  { poss1_blue, "poss1_blue" },
  { quickv, "quickv" },
  { phase2_gsc2, "phase2_gsc2" },
  { phase2_gsc1, "phase2_gsc1" },
};


DSSImage::Private::Private( const Coordinates::Equatorial &coordinates, const Angle &size, DSSImage::ImageVersion imageVersion, DSSImage *q ) 
  : coordinates(coordinates), size(size), imageVersion(imageVersion), q( q )
{
  string cacheDir("dss-cache");
  wApp->readConfigurationProperty("dss-cache-dir", cacheDir);
  fs::create_directories(cacheDir);
  cacheFile = fs::path(cacheDir) / cacheKey();
}

DSSImage::~DSSImage()
{
}


DSSImage::ImageVersion DSSImage::imageVersion( const string &version )
{
  for(auto v: Private::imageVersionStrings) {
    if(version == v.second)
      return v.first;
  }
  return phase2_gsc2;
}

string DSSImage::imageVersion( const DSSImage::ImageVersion &version )
{
  return Private::imageVersionStrings[version];
}

vector< DSSImage::ImageVersion > DSSImage::versions()
{
  vector<ImageVersion> v;
  transform(begin(Private::imageVersionStrings), end(Private::imageVersionStrings), back_inserter(v), [](pair<ImageVersion,string> p) { return p.first; });
  return v;
}



string DSSImage::Private::cacheKey() const
{
  return format("%s-ar_%d-%d-%.1f_dec_%d-%d-%.1f_size_%d-%d-%.1f.gif")
  % imageVersionStrings[imageVersion]
  % coordinates.rightAscension.sexagesimalHours().hours
  % coordinates.rightAscension.sexagesimalHours().minutes
  % coordinates.rightAscension.sexagesimalHours().seconds
  % coordinates.declination.sexagesimal().degrees
  % coordinates.declination.sexagesimal().minutes
  % coordinates.declination.sexagesimal().seconds
  % size.sexagesimal().degrees
  % size.sexagesimal().minutes
  % size.sexagesimal().seconds;
}
string DSSImage::Private::imageLink() const
{
  Angle::Sexagesimal objectSize = size.sexagesimal();
  double objectRect = static_cast<double>(objectSize.degrees * 60) + static_cast<double>(objectSize.minutes) + (objectSize.seconds/60);
  double multiplyFactor = 2.5;
  if(objectRect < 20)
    multiplyFactor = 3.5;
  if(objectRect < 10)
    multiplyFactor = 4.5;
  if(objectRect < 5 )
    multiplyFactor = 6.;
  if(objectRect < 1 )
    multiplyFactor = 10.;
  
  objectRect = min(75.0, objectRect * multiplyFactor);
  objectRect = objectRect <= 0 ? 75.0:objectRect; // objects without angular size (-1), showing max possible field...

  return format("http://archive.stsci.edu/cgi-bin/dss_search?v=%s&r=%d+%d+%.1f&d=%d+%d+%.1f&e=J2000&h=%d&w=%df&f=gif&c=none&fov=SM97&v3=")
  % imageVersionStrings[imageVersion]
  % coordinates.rightAscension.sexagesimalHours().hours
  % coordinates.rightAscension.sexagesimalHours().minutes
  % coordinates.rightAscension.sexagesimalHours().seconds
  % coordinates.declination.sexagesimal().degrees
  % coordinates.declination.sexagesimal().minutes
  % coordinates.declination.sexagesimal().seconds
  % objectRect % objectRect;
}

void DSSImage::Private::setCacheImage()
{
  content->clear();
  auto *resource = new WFileResource(cacheFile.string(), q);
  auto anchor = new WAnchor(resource);
  anchor->setTarget(Wt::TargetNewWindow);
  anchor->addWidget(new WImage(resource));
  content->addWidget(anchor);
}
void DSSImage::Private::startDownload()
{
  content->clear();
  content->addWidget(new WText("Downloading image from DSS Website, wait..."));
  Http::Client *client = new Http::Client(q);
  client->setTimeout(120);
  client->setMaximumResponseSize(1024 * 1024 * 50);
  client->get(imageLink());
  WApplication *app = wApp;
  client->done().connect([=](boost::system::error_code err, Http::Message message, _n4){
    Scope triggerUpdate{[=]{app->triggerUpdate();}};
    wApp->log("notice") << __PRETTY_FUNCTION__ << ", download done: err=" << err  << "(" << err.message() << ")" << ", status: " << message.status() << ", retry: " << retry;
    for(auto header: message.headers()) {
      wApp->log("notice") << __PRETTY_FUNCTION__ << " message header: " << header.name() << ", value='" << header.value() << "'";
    }
    const string *contentType = message.getHeader("Content-Type");
    auto isNotAnImage = [=] {
      return !contentType || contentType->find("image/") == string::npos; 
    };
    if( (err  || message.status() != 200 || isNotAnImage() ) && retry < 3 ) {
      client->get(imageLink());
      retry++;
      return;
    }
    if( err || message.status() != 200 || isNotAnImage() ) {
      content->addWidget(
        new WText{WString("<br />Error retrieving DSS Image: {1}<br />Please try another format, or click the link above to try viewing it in the DSS Archive.")
        .arg(err ? err.message() : WString("HTTP Status {1}").arg(message.status() ))});
      return;
    }
    ofstream s(cacheFile.string());
    s << message.body();
    s.close();
    setCacheImage();
  });
}

DSSImage::DSSImage( const Coordinates::Equatorial &coordinates, const Angle &size, DSSImage::ImageVersion imageVersion, WContainerWidget *parent )
  : WCompositeWidget(parent), d( coordinates, size, imageVersion, this )
{
  d->content = new WContainerWidget;
  WAnchor *original = new WAnchor(d->imageLink(), "Original DSS Image Link");
  original->setInline(false);
  original->setTarget(Wt::TargetNewWindow);
  setImplementation(WW<WContainerWidget>().add(original).add(d->content));
  if(fs::exists(d->cacheFile))
    d->setCacheImage();
  else
    d->startDownload();
}
